//----------------------------------------------------------------------------
// For simplicity of generation, here are registered ouput signals of tmaxs
//----------------------------------------------------------------------------
fprintf(vfp, "reg [15:0] clP0_cnt, clP1_cnt, clP2_cnt, clP3_cnt;\n");
fprintf(vfp, "reg [15:0] scP_cnt, lnScaler_cnt;\n\n");
fprintf(vfp, "always @ (posedge clk) begin   \n");
fprintf(vfp, "  if (rst) begin               \n");
fprintf(vfp, "    clP0_cnt <= 0;             \n");
fprintf(vfp, "    clP1_cnt <= 0;             \n");
fprintf(vfp, "    clP2_cnt <= 0;             \n");
fprintf(vfp, "    clP3_cnt <= 0;             \n");
fprintf(vfp, "    scP_cnt  <= 0;             \n");
fprintf(vfp, "    lnScaler_cnt <= 0;         \n");
fprintf(vfp, "  end                          \n");
fprintf(vfp, "  if (!stall && tmax_fadd1_res_rdy) begin  \n");
fprintf(vfp, "    clP0_cnt <= clP0_cnt + 1;  \n");
fprintf(vfp, "  end                          \n");
fprintf(vfp, "  if (!stall && tmax_fmul1_res_rdy) begin  \n");
fprintf(vfp, "    clP1_cnt <= clP1_cnt + 1;  \n");
fprintf(vfp, "  end                          \n");
fprintf(vfp, "  if (!stall && tmax_fdiv1_res_rdy) begin  \n");
fprintf(vfp, "    clP2_cnt <= clP2_cnt + 1;  \n");
fprintf(vfp, "  end                          \n");
fprintf(vfp, "  if (!stall && tmax_fsub1_res_rdy) begin  \n");
fprintf(vfp, "    clP3_cnt <= clP3_cnt + 1;  \n");
fprintf(vfp, "  end                          \n");
fprintf(vfp, "  if (!stall && tmax_fmax1_res_rdy) begin   \n");
fprintf(vfp, "    scP_cnt <= scP_cnt + 1;    \n");
fprintf(vfp, "  end                          \n");
fprintf(vfp, "  if (!stall && tmax_fgt1_res_rdy) begin   \n");
fprintf(vfp, "    lnScaler_cnt <= lnScaler_cnt + 1;      \n");
fprintf(vfp, "  end                                      \n");
fprintf(vfp, "end                                        \n\n");
fprintf(vfp, "always @ (posedge clk) begin                      \n");
fprintf(vfp, "  tmax_fadd1_out_rdy <= ~rst & tmax_fadd1_res_rdy;\n");
fprintf(vfp, "  tmax_fadd1_out <=        tmax_fadd1_res;        \n");
fprintf(vfp, "  tmax_fmul1_out_rdy <= ~rst & tmax_fmul1_res_rdy;\n");
fprintf(vfp, "  tmax_fmul1_out <=        tmax_fmul1_res;        \n");
fprintf(vfp, "  tmax_fdiv1_out_rdy <= ~rst & tmax_fdiv1_res_rdy;\n");
fprintf(vfp, "  tmax_fdiv1_out <=        tmax_fdiv1_res;        \n");
fprintf(vfp, "  tmax_fsub1_out_rdy <= ~rst & tmax_fsub1_res_rdy;\n");
fprintf(vfp, "  tmax_fsub1_out <=        tmax_fsub1_res;        \n");
fprintf(vfp, "  tmax_fmax1_out_rdy <= ~rst & tmax_fmax1_res_rdy;\n");
fprintf(vfp, "  tmax_fmax1_out <=        tmax_fmax1_res;        \n");
fprintf(vfp, "  tmax_fgt1_out_rdy  <= ~rst & tmax_fgt1_res_rdy; \n");
fprintf(vfp, "  tmax_fgt1_out  <=        tmax_fgt1_res;         \n");
fprintf(vfp, "end                                               \n\n");

