float sample8(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = (ports[0].f * 3 + (ports[1].f * 0.5 + 2 + 4 * ports[0].f * ports[2].f) + 0.5) /           ((ports[0].f + 0.5) * 4 + ports[0].f * 4 + ports[1].f * 0.5 + ports[2].f * 3 + 0.5);
  printf("Single sample8 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}
