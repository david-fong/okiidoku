#ifndef HPP_OKIIDOKU_UTIL__PROB
#define HPP_OKIIDOKU_UTIL__PROB

[[nodiscard, gnu::const]] constexpr unsigned long long n_choose_r(unsigned int n, unsigned int r) noexcept {
	if (r > n - r) r = n - r;
	unsigned long long ans = 1;
	for (unsigned int i = 1; i <= r; ++i) {
		ans *= n - r + i;
		ans /= i;
	}
	return ans;
}
#endif