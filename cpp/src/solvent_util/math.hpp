#ifndef HPP_SOLVENT_UTIL__PROB
#define HPP_SOLVENT_UTIL__PROB

constexpr inline unsigned long n_choose_r(unsigned int n, unsigned int r) {
	if (r > n - r) r = n - r;
	unsigned long ans = 1;
	for (unsigned int i = 1; i <= r; i++) {
		ans *= n - r + i;
		ans /= i;
	}
	return ans;
}
#endif