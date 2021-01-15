//Fluffy: For miscellanous, generic snippets of code

int InterpolateBetweenTwoPoints_Int32(int prev, int next, double progress)
{
	if (prev == next)
		return next;

	return prev + (progress * (next - prev));
}
