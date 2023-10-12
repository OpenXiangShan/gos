int strncmp(const char *cs, const char *ct, int count)
{
	unsigned char c1, c2;

	while (count) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
		count--;
	}

	return 0;
}

int strcpy(char *dst, char *src)
{
	char *tmp = dst;

	if (!src)
		return -1;

	while (*src)
		*tmp++ = *src++;

	return 0;
}
