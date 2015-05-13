float sample9(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = (ports[0].f * 3 + (ports[1].f * 0.5 + 3 + 4 * ports[0].f * ports[2].f + ports[3].f * 2 + ports[4].f * 3) + 0.5) /           ((ports[0].f + 0.5) * 4 + ports[0].f * 4 + ports[5].f * 0.5 + ports[2].f * 3 + 0.5 + ports[3].f * 0.5 + ports[4].f * 0.5);
  printf("Single sample9 res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}
