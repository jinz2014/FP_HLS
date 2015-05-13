float plf(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = ((ports[0].f * 0.1 + ports[1].f * 0.2) + (ports[2].f * 0.3 + ports[3].f * 0.4)) * ((ports[4].f * 0.5 + ports[5].f * 0.6) + (ports[6].f * 0.7 + ports[7].f * 0.8));
  printf("Single plf res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}
