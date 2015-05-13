float sample7(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = (ports[0].f + 1.0) * (ports[1].f + 1.0) * (ports[2].f + 1.0);
  printf("Single sample7 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}
