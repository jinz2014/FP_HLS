assign go = ~stall_in & go_in;

always @ (posedge clk or posedge rst)
  begin
    if (rst)
      run <= 1'b0; 
    else if (go)
      run <= 1'b1;
  end


// After all the input data have been consumed, state_wen is still asserted.
// When stall(stall_in | finish) is asserted, ShiftRegister is not enabled.
assign state_wen = (go_in | run) & ~stall;

// DII interval Entry
ShiftCtlRegister #(1, Delay) FSM (
  .clk  (clk      ),
  .rst  (rst),
  .en   (state_wen),
  .din  (state_din),
  .dout (state_out),
  .wdout(state)
);
