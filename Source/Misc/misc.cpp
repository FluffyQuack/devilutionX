//Fluffy: For miscellanous, generic snippets of code

#include "../all.h"

DEVILUTION_BEGIN_NAMESPACE

int InterpolateBetweenTwoPoints_Int32(int prev, int next, double progress)
{
	if (prev == next)
		return next;

	return prev + (progress * (next - prev));
}

DEVILUTION_END_NAMESPACE
