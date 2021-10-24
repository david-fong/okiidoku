#ifndef HPP_SOLVENT_UTIL_PROB
#define HPP_SOLVENT_UTIL_PROB

constexpr inline unsigned long n_choose_r(int n, int r) {
	if (r > n - r) r = n - r;
	unsigned long ans = 1;
	for (unsigned int i = 1; i <= r; i++) {
		ans *= n - r + i;
		ans /= i;
	}
	return ans;
}
#endif