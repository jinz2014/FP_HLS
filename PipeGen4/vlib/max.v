/*
 * Author JZ 
 *
 * 8/17/11
 * output register stage's control enable is (rdy_out && pipeEn)
 */

module TestMax;

parameter DataWidth = 32; 

// 3 is default latency of less-than operator in Xilinx library
parameter Depth = 3; 

reg clk;
reg pipeEn;
reg go;
reg  [DataWidth - 1 : 0] a, b;
wire [DataWidth - 1 : 0] result;

// DUT
fmax #(DataWidth, Depth) fmaxi 
(
  .clk(clk),
  .a(a),
  .b(b),
  .go(go),
  .result(result),
  .pipeEn(pipeEn)
);

always #5 clk = ~clk;

initial begin
  clk = 0;

  #100

  go = 1'b1;
  pipeEn = 1'b1;
  @(negedge clk);
  a = 32'h3f6d24f7; b = 32'h3fdbad77; 
  @(negedge clk);
  a = 32'h3f06c8af; b = 32'h3f0ded69; 
  @(negedge clk);
  a = 32'h3e9e9b9a; b = 32'h3e4488e1; 
  @(negedge clk);
  a = 32'h3f0090de; b = 32'h3f012260; 

  #100;
  $finish;
end

reg [31:0] gold_result;

// 
always @ (a or b) begin
  gold_result = a > b ? a : b;
end

endmodule 
// synthesis translate_on

//---------------------------------------------- 
// Parameterized max function operator 
//
// res = a > b ? a : b
// 
//
// Single floating-point max fmax
// Double floating-point max fmaxd
//
// parameter: Data Width, Depth
//---------------------------------------------- 


module fmax(clk, a, b, go, result, pipeEn, rdy);

  parameter DataWidth = 32;
  parameter Depth = 1;

  input                       clk;
  input  [DataWidth - 1 : 0]  a, b;
  input                       go;
  input                       pipeEn;
  output reg [DataWidth - 1 : 0]  result; 
  output reg                  rdy; 

  wire [DataWidth - 1 : 0]  a_dly, b_dly;
  wire [DataWidth * Depth - 1 : 0]      a_wdout, b_wdout;
  wire flt1_out;
  wire rdy_out;

  // call xilinx less-than floating-point operator
  flt flt1 (
    .clk(clk),
    .a(a),
    .b(b),
    .go(go),
    .result(flt1_out),
    .pipeEn(pipeEn),
    .rdy(rdy_out)
  );

  // delay inputs a and b by the latency of less-than floating-point operator
  ShiftRegister #(DataWidth, Depth) ina (clk, pipeEn, a, a_dly, a_wdout);
  ShiftRegister #(DataWidth, Depth) inb (clk, pipeEn, b, b_dly, b_wdout);

  always @ (posedge clk) begin
    if (rdy_out && pipeEn) begin 
      result <= flt1_out ? b_dly : a_dly;
      rdy    <= rdy_out; 
    end
  end

endmodule

//=====================================================================
//
//=====================================================================
module fmaxd(clk, a, b, go, result, pipeEn, rdy);

  parameter DataWidth = 64;
  parameter Depth = 2;

  input                       clk;
  input  [DataWidth - 1 : 0]  a, b;
  input                       go;
  input                       pipeEn;
  output reg [DataWidth - 1 : 0]  result; 
  output reg                  rdy; 

  wire [DataWidth - 1 : 0]  a_dly, b_dly;
  wire flt1_out;
  wire rdy_out;

  // call xilinx less-than floating-point operator
  fltd flt1 (
    .clk(clk),
    .a(a),
    .b(b),
    .go(go),
    .result(flt1_out),
    .pipeEn(pipeEn),
    .rdy(rdy_out)
  );

  // delay inputs a and b by the latency of less-than floating-point operator
  ShiftRegister #(DataWidth, Depth) ina (clk, pipeEn, a, a_dly);
  ShiftRegister #(DataWidth, Depth) inb (clk, pipeEn, b, b_dly);

  always @ (posedge clk) begin
    if (rdy_out && pipeEn) begin
      result <= flt1_out ? b_dly : a_dly;
      rdy    <= rdy_out; 
    end
  end

endmodule
