unsigned long pow(unsigned int x, unsigned int N)
{
	unsigned long ret = 1;
	int i;

	for (i = 0; i < N; i++)
		ret *= x;

	return ret;
}
