
#include "pavillon.h"

void	ft_bzero(unsigned char *s, size_t n)
{
	unsigned char	*txt;
	int				a;
	int				b;

	txt = s;
	a = 0;
	b = n;
	while (a < b)
	{
		txt[a] = 0;
		a++;
	}
}
