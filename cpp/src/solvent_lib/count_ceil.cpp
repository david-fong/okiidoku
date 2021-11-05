#include <solvent_lib/count_ceil.hpp>

/* note: a trick for comparing products without calculating a
potentially very large-valued product: take a list of numbers
that you would otherwise calculate the large product for, and
create an array where indices are each for a prime factor,
and the entry value is the occurrence-count of that factor in
the number list. Then, still calculate the product, but first,
between the two factorization arrays, subtract from each entry
at index i the min of their values.
*/