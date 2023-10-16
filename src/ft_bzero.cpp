/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_bzero.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: r <r@student.42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/10/24 17:54:08 by rdecelie          #+#    #+#             */
/*   Updated: 2023/10/17 00:57:56 by r                ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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


bool copyFileToSD(const char* sourceFileName, const char* targetFileName) {
  File sourceFile = SD.open(sourceFileName);
  if (!sourceFile) {
    Serial.println("Impossible d'ouvrir le fichier source.");
    return false;
  }

  File targetFile = SD.open(targetFileName, FILE_WRITE);
  if (!targetFile) {
    Serial.println("Impossible d'ouvrir le fichier cible.");
    sourceFile.close();
    return false;
  }

  while (sourceFile.available()) {
    char data = sourceFile.read();
    targetFile.write(data);
  }

  sourceFile.close();
  targetFile.close();
  return true;
}
