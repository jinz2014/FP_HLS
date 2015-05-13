module Register(
   clk,
   rst,
   wen,
   wdata, 
   rdata);
 
  parameter DataWidth = 32;

  input clk;
  input rst;
  input wen;                    // Reg Write enable
  input [DataWidth-1:0] wdata;  // Reg Write port  
  output [DataWidth-1:0] rdata; // Reg Read data port 1

  reg [DataWidth-1:0] register;
  
  /*
  always @(posedge clk) begin
    register <= rst ? {DataWidth{1'b0}} : wen ? wdata : register;
  end
  */

  always @(posedge clk) begin
    if (rst)
      register <= {DataWidth{1'b0}};
    else if (wen)
      register <= wdata;
  end
 
  assign rdata = register;

endmodule 
/*
module Register(
   clk,
   rst,
   stall,
   wen,
   wdata, 
   rdata);
 
  parameter DataWidth = 32;

  input clk;
  input rst;
  input stall;
  input wen;                    // Reg Write enable
  input [DataWidth-1:0] wdata;  // Reg Write port  
  output [DataWidth-1:0] rdata; // Reg Read data port 1

  reg [DataWidth-1:0] register;
  
  always @(posedge clk or posedge rst) begin
    if (rst)
      register <= {DataWidth{1'b0}};
    else if (!stall) begin
    //else if (!0) begin
      if (wen) register <= wdata;
    end
  end
 
  assign rdata = register;

endmodule 
*/


module RegisterV(
   clk,
   rst,
   stall,
   clr,
   wen,
   wdata, 
   rdata,
   rdy);
 
  parameter DataWidth = 32;

  input clk;
  input rst;
  input stall;                  // Stall 
  input clr;                    // Clear
  input wen;                    // Write enable
  input  [DataWidth-1:0] wdata; // Write port  
  output [DataWidth-1:0] rdata; // Read data port 1
  output                 rdy;   // Read data valid

  reg [DataWidth-1:0] register;
  reg                 ready;
  
  always @(posedge clk or posedge rst) begin
    if(rst) begin
      register <= {DataWidth{1'b0}};
      ready      <= 1'b0; 
    end
    else if (!stall) begin
      if (wen) begin
        register <= wdata;
        ready    <= 1'b1; 
      end 
      else if (clr)
        ready    <= 1'b0;
    end
  end
 
  assign rdata = register;
  assign rdy   = ready;

endmodule 

module RegisterC(
   clk,
   rst,
   stall,
   wdata, 
   rdata);
 
  parameter DataWidth = 32;

  input clk;
  input rst;
  input stall;
  input [DataWidth-1:0] wdata;  // Reg Write port  
  output [DataWidth-1:0] rdata; // Reg Read data port 1

  reg [DataWidth-1:0] register;
  
  always @(posedge clk or posedge rst) begin
    if (rst)
      register <= {DataWidth{1'b0}};
    else if (!stall)
      register <= wdata;
  end
 
  assign rdata = register;

endmodule 
