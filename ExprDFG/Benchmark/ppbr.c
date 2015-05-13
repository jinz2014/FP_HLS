float ppbr(FILE *fp, int32_or_float* ports, int maxPortPair) {
  int32_or_float res;
  res.f = ports[0].f * (ports[1].f * ports[2].f + ports[3].f * ports[4].f / ports[5].f) /           ((ports[1].f * ports[2].f + ports[6].f * ports[1].f + ports[7].f * ports[2].f * (-1 + ports[4].f / ports[8].f)) +           ports[0].f / (ports[9].f * ports[5].f) * (ports[10].f * ports[3].f * (-1 + ports[1].f / ports[11].f) +            ports[4].f * (ports[12].f + ports[3].f)));
  printf("Single ppbr res=%.11f\n", res.f);
  fprintf(fp, "%08x\n", res.i);
  return res.i;
}
