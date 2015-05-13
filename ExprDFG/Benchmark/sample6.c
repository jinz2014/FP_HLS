float sample6(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = (ports[0].f - (ports[0].f + 2 - 4 * ports[1].f * ports[2].f) + 0.5) / (2 * ports[1].f);
  printf("Single sample6 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}
