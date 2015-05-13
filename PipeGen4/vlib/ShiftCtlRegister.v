// version 1 register
module ShiftCtlRegister(clk, rst, en, din, dout, wdout);

parameter DataWidth = 8;
parameter Depth = 4;

wire DBG = 0;
input                              clk;
input                              rst;
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
  shiftCtl  #(Depth, 0) shiftCtl_i (clk, rst, en, din[i], dout[i], 
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

module shiftCtl (clk, rst, en, bin, bout, state);
  parameter n = 4;  // 1-bit register with depth = 4
  parameter Pattern = 0;  
  input          clk;
  input          rst;
  input          en;
  input          bin;
  output         bout;
  output [n-1:0] state;

  reg    [n-1:0] state;

  always @ (posedge clk) begin
    if (rst) begin
      state <= Pattern;
    end
    else if (en) begin
      // one cycle delay relative to state output
      //bout  <= state[n - 1]; 
      state <= {state[n-2:0], bin};
    end
  end

  assign bout = state[n - 1]; // MSB

endmodule

// version 2  memory 



