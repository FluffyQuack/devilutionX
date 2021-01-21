//Fluffy: For miscellanous, generic snippets of code

#include "..\all.h"

DEVILUTION_BEGIN_NAMESPACE

#define PI 3.14159265358979323846
void RotateVector(double angle, double *x, double *y) //Angle should be between 0 and 360
{
	double newX = *x * cos(angle * PI / 180.0f) - *y * sin(angle * PI / 180.0f);
	double newY = *x * sin(angle * PI / 180.0f) + *y * cos(angle * PI / 180.0f);
	*x = newX;
	*y = newY;
}

int InterpolateBetweenTwoPoints_Int32(int prev, int next, double progress)
{
	if (prev == next)
		return next;

	return prev + (progress * (next - prev));
}

DEVILUTION_END_NAMESPACE
