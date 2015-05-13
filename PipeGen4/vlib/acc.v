// synopsys translate_off

module TB ();

  reg  [0 : 0]  clk;
  reg  [0 : 0]  rst;
  reg  [63 : 0] acc_in;
  reg  [0 : 0]  acc_in_valid;
  wire [63 : 0] acc_out;
  wire [0 : 0]  acc_out_valid;

  // Number of inputs to be accumulated
  parameter N = 5000;

  // Output latency which is larger than or
  // smaller than the FADDD's latency
  parameter LATENCY = 21;

  integer i;
  integer cnt = 0;
  reg [63:0] acc_res;

  real     acc;

  acc acc_i (
    clk,
    rst,
    acc_in,
    acc_in_valid,
    acc_out,
    acc_out_valid
  );

  always #10 clk = ~clk;


  initial begin
    acc = 0;
    for (i = 1; i <= N; i=i+1) begin
      acc = acc + i * 0.001;
    end
  end

  initial begin
    clk = 0;
    rst = 0;
    acc_in_valid = 1'b0;
    acc_in       = 64'b0;

    #50 rst = 1;
    #80 rst = 0;

    for (i = 1; i <= N; i=i+1) begin
      repeat (LATENCY) @(posedge clk);
      begin
        acc_in = $realtobits(i * 0.001); 
        acc_in_valid = 1'b1;
      end
      @(posedge clk) begin
        acc_in_valid = 1'b0;
      end
    end

    #2000; // wait for hw delay

  end

  // register hw result in Test Bench and count the test number
  always @ (posedge clk) begin
    if (acc_i.sum_out_valid) begin
      acc_res = acc_out; 
      cnt = cnt + 1;  // Note there are N - 1 additions for N numbers
      //$display("sim: sum = %h(%f)", acc_out, $bitstoreal(acc_out));
    end
  end

  // show accumulation result
  always @ (*) begin
    if (cnt == N - 1) begin 
      $display("sim: acc = %h(%f)", acc_res, $bitstoreal(acc_res));
      $display("exp: acc = %h(%f)", $realtobits(acc), acc);
      $finish;
    end
  end

endmodule

// synopsys translate_on

module acc (
  clk,
  rst,
  acc_in,
  acc_in_valid,
  acc_out,
  acc_out_valid
);

  input  [0 : 0]  clk;
  input  [0 : 0]  rst;
  input  [63 : 0] acc_in;
  input  [0 : 0]  acc_in_valid;
  output [63 : 0] acc_out;
  output [0 : 0]  acc_out_valid;

  wire [63 : 0] reg_data_in;
  wire [63 : 0] mux0_out, mux1_out, mux2_out;
  wire [63 : 0] sum_out;
  wire [13 : 0] wdout;
  wire          sel0, sel1, sel2;
  wire          sum_in_valid, sum_out_valid;
  wire          reg_data_valid_clr;
  wire          reg_data_en;
  wire          acc_rdy;

  reg  [63 : 0] reg_data;
  reg           reg_data_valid;
  reg  [5 : 0]  vec;

  always @ (posedge clk) begin
    if (reg_data_valid_clr || rst)
      reg_data_valid <= 1'b0;

    else if (reg_data_en)
      reg_data_valid <= 1'b1;
  end

  always @ (posedge clk) begin
    if (reg_data_en)
      reg_data <= reg_data_in;
  end

  assign reg_data_in = mux0_out;

    /*
  always @ (posedge clk)
    if (fadd_en)
      fadd_vbuf_out <= fadd_vbuf_in;
    */

  assign reg_data_valid_clr = vec[5]; 
  assign sum_in_valid       = vec[4]; 
  assign reg_data_en        = vec[3]; 
  assign sel0               = vec[2]; 
  assign sel1               = vec[1]; 
  assign sel2               = vec[0]; 


  assign mux0_out = sel0 ? sum_out : acc_in;
  assign mux1_out = sel1 ? sum_out : acc_in;
  assign mux2_out = sel2 ? acc_in  : reg_data;

  assign acc_out        = sum_out;
  assign acc_out_valid  = acc_rdy;  // how mand data to be accumulated?

  always @ (*) begin
      //-------------------------------------------
      // Accumulator controller
      //
      // output vector bits
      //
      // b5: reg_out_valid_clk
      // b4: sum_valid
      // b3: reg_data_en
      // b2: sel0
      // b1: sel1
      // b0: sel2
      //-------------------------------------------
      case ({acc_in_valid, reg_data_valid, sum_out_valid})
        3'd0: vec = 6'b000_xxx;
        3'd1: vec = 6'b001_1xx;
        3'd2: vec = 6'b000_xxx;
        3'd3: vec = 6'b110_x10;
        3'd4: vec = 6'b001_0xx;
        3'd5: vec = 6'b010_x11;
        3'd6: vec = 6'b110_x00;
        3'd7: vec = 6'b011_010;
        default: vec = 6'b000000;
      endcase
    end

  ShiftRegister #(1, 14) SR_valid 
  (clk, 1'b1, sum_in_valid, sum_out_valid, wdout);

  // 14-latency
  faddd faddd1 
  (clk, mux2_out, mux1_out, 1'b1, sum_out, acc_rdy, 1'b1);

endmodule
