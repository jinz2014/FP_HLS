task test;

  input   [5:0] cycles;
  input         stall;
  input  [31:0] in0;
  //input  [31:0] in1;

  output        stim_stall;
  output        stim_go;
  output [31:0] stim_in0;
  //output [31:0] stim_in1;

  if (stall) begin
    repeat (cycles) begin
      #1
      stim_stall = 1'b1;
      stim_go    = 1'b0;
      stim_in0   = in0;
   //   stim_in1   = in1;
    end
  end
  else begin
    repeat (cycles) begin 
      #1
      stim_stall = 1'b0;
      stim_go    = 1'b1;
      stim_in0   = in0;
    //  stim_in1   = in1;
    end
  end
endtask

