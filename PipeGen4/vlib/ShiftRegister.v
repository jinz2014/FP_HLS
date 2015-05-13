// synthesis translate_off

module TestShiftRegister;

parameter DataWidth = 32;
parameter Depth = 3;
reg clk, en;
reg [DataWidth - 1 : 0]         din, dout;
reg [DataWidth * Depth - 1 : 0] wdout;

integer i;

ShiftRegister #(DataWidth, Depth) SRi (clk, en, din, dout, wdout);

always #5 clk = ~clk;

initial begin
  clk = 0;
  en  = 0;
  din = {DataWidth{1'bx}};

  #100
  en = 1;

  for (i = 1; i <= Depth * 2; i = i + 1) begin
    @(negedge clk) din = i;
    $display("%0d din = %h", $time, din);
  end

  #100
  $finish;
end

endmodule 
// synthesis translate_on

//---------------------------------------------- 
// Parameterized shift register
// 
// parameter: Data Width, Depth
//---------------------------------------------- 


module ShiftRegister(clk, en, din, dout, wdout);

parameter DataWidth = 8;
parameter Depth = 4;

wire DBG = 0;
input                              clk;
input                              en;

// shift data in
input  [DataWidth - 1 : 0]         din;

// shift data out
output [DataWidth - 1 : 0]         dout; 

// shift wide data out
output [DataWidth * Depth - 1 : 0] wdout; 

wire   [DataWidth * Depth - 1 : 0] temp; 

genvar i; 

//----------------------------------------------------------------
//  Convert temp to wdout
//
// e.g. DataWidth = 3, Depth = 2
//
//     temp[1] temp[0]   // 1-bit shift_i
//         d0  d1
//
//     temp[3] temp[2]   // 1-bit shift_i
//         d3  d2
//
//     temp[5] temp[4]   // 1-bit shift_i
//         d5  d4
//
//  wdout[2:0] = {d4, d2, d1}
//  wdout[5:3] = {d5, d3, d0}
//----------------------------------------------------------------
generate for (i = 0; i < DataWidth; i = i + 1) begin : SR
  shift #(Depth, 0) shift_i (clk, en, din[i], dout[i], 
                          temp[(i + 1) * Depth - 1 : i * Depth]); 
end endgenerate

generate for (i = 0; i< DataWidth * Depth; i = i + 1) begin : WD
  assign wdout[i] = temp[i % DataWidth * Depth + i / DataWidth];
end endgenerate

// synthesis translate_off
always @ (wdout, dout)
  if (DBG) $display("%0d wdout = %h dout = %h", 
    $time, wdout[DataWidth * Depth - 1 : 0], dout);
// synthesis translate_on
   
endmodule

module shift(clk, en, bin, bout, state);
  parameter n = 4;  // 1-bit shift register with depth = 4
  parameter Pattern = 0;  
  input          clk;
  input          en;
  input          bin;
  output         bout;
  output [n-1:0] state;

  //reg            bout;
  reg    [n-1:0] state = Pattern; //{n{1'b0}};

  always @ (posedge clk)
  begin
    if (en) begin
      // one cycle delay relative to state output
      //bout  <= state[n - 1]; 
      state <= {state[n-2:0], bin};
    end
  end

  assign bout = state[n - 1]; // MSB

endmodule

//-------------------------------------------------------------
// Control shift register
//-------------------------------------------------------------
/*
module ShiftRegCtl(clk, en, 
                        //din, 
                        dout);
                        //wdout);

parameter DataWidth = 8;
parameter Depth     = 4;
parameter Pattern   = 3;

wire DBG = 0;
input                              clk;
input                              en;

// shift data in
//input  [DataWidth - 1 : 0]         din;

// shift data out
output [DataWidth - 1 : 0]         dout; 

// shift wide data out
//output [DataWidth * Depth - 1 : 0] wdout; 
wire [DataWidth * Depth - 1 : 0] wdout; 

reg  shift_en = 0;
wire dio;

  //shift_ctl shift_ctl_i (clk, en, shift_en);
  always @ (posedge clk)
  begin
    if (en) begin
      shift_en <= 1'b1;
    end
  end

  // 1-bit cyclic shift
  shift #(Depth, Pattern) shift_i (clk, shift_en, dio, 
                          dio, wdout);

assign dout =  shift_en & wdout[0];

// synthesis translate_off
always @ (wdout, dout)
  if (DBG) $display("%0d wdout = %h dout = %h", 
    $time, wdout, dout);
// synthesis translate_on

endmodule
*/

module ShiftRegCtl(clk, 
                   stall, 
                   en, 
                   dout);
                        //wdout);

parameter DataWidth = 8;
parameter Depth     = 4;
parameter Pattern   = 3;

wire DBG = 0;
input                              clk;
input                              stall;
input                              en;

// shift data in
//input  [DataWidth - 1 : 0]         din;

// shift data out
output [DataWidth - 1 : 0]         dout; 

// shift wide data out
//output [DataWidth * Depth - 1 : 0] wdout; 
wire [DataWidth * Depth - 1 : 0] wdout; 

reg  shift_en = 0;
wire dio;

  //shift_ctl shift_ctl_i (clk, en, shift_en);
  always @ (posedge clk)
  begin
    if (stall)
      shift_en <= 1'b0;
    else if (en)
      shift_en <= 1'b1;
  end

  // 1-bit cyclic shift
  shift #(Depth, Pattern) shift_i (clk, shift_en, dio, 
                          dio, wdout);

assign dout =  shift_en & wdout[0];

// synthesis translate_off
always @ (wdout, dout)
  if (DBG) $display("%0d wdout = %h dout = %h", 
    $time, wdout, dout);
// synthesis translate_on

endmodule


  
module ShiftRegCtl0(clk, 
                    stall,
                    en, 
                    //din, 
                    dout);
                    //wdout);

parameter DataWidth = 8;
parameter Depth     = 4;
parameter Pattern   = 3;

wire DBG = 0;
input                              clk;
input                              stall;
input                              en;

// shift data out
output [DataWidth - 1 : 0]         dout; 

// shift wide data out
//output [DataWidth * Depth - 1 : 0] wdout; 
wire [DataWidth * Depth - 1 : 0] wdout; 

wire shift_en;
wire dio;

reg  state = 1'b0;


  //shift_ctl shift_ctl_i (clk, en, shift_en);
  always @ (posedge clk)
  begin
    if (en) begin
      state <= 1'b1;
    end
  end

  assign shift_en = ~stall & (state | en);

  // 1-bit cyclic shift
  shift #(Depth, Pattern) shift_i (clk, shift_en, dio, 
                          dio, wdout);

assign dout =  shift_en & wdout[0];

// synthesis translate_off
always @ (wdout, dout)
  if (DBG) $display("%0d wdout = %h dout = %h", 
    $time, wdout, dout);
// synthesis translate_on

endmodule

/* cyclic shift enable
module shift_ctl(clk, en, bout);
  //parameter n = 2;  // 1-bit shift register with depth = 4
  input          clk;
  input          en;
  output         bout;
  //reg    [n-1:0] state = 2'b0;
  reg            state = 1'b0;

  //reg            bout;

  always @ (posedge clk)
  begin
    if (en) begin
      state <= 1'b1;
    end
  end

  assign bout = state | en;

endmodule
*/

