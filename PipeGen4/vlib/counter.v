`define IDLE 0 
`define RUN 1

module FinishCounter(clk, rst, stall, enable, finish);
  input clk;
  input rst;
  input stall;
  input enable;
  output finish;

  parameter Delay = 32;
  parameter CntWidth = 5;

  reg [CntWidth - 1 : 0] Count;
  reg [1 : 0 ]           State;
  wire                   stop;

  always @ (posedge clk) begin
    if (rst) 
      State <= 2'b0;  // IDLE
    else if (enable) 
      State <= 2'b1;  // RUN
  end
  
  always @ (posedge clk) begin
    if (rst) 
      Count <= 0;
    else if (!stall && !stop && (State || enable)) begin
      Count <= Count + 1;
    end
  end

  assign stop = (Count == Delay) ? 1'b1 : 1'b0;
  assign finish = stop;

endmodule
