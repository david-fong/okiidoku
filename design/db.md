# Brainstorming Database Ideas

## Compression

For full grids, the blocks along the main diagonal can be removed and are easy to restore.

### Compression by House

To store an ordered sequence of numbers in `[0 N)`, one can map the sequence to a version where each entry is bumped down by the number of receding values that are less than its value. Then reduce the array to a number in a digit system where each place stores `N - index` values. The maximum value is `N! - 1`.

You can plug the following into desmos to see for yourself (Note that the factorial gets too big for desmos to handle before you can see the naive size/cost jump up at `o = 16`):

```latex
o_{seq}=\left[0...20\right]

B_{house}\left(o\right)=\operatorname{floor}\left(\frac{7+\left(1+\operatorname{floor}\left(\log_{2}\left(o^{2}!-1\right)\right)\right)}{8}\right)\left\{o>1\right\}

B_{naive}\left(o\right)=o^{2}\cdot\operatorname{floor}\left(\frac{7+\left(1+\operatorname{floor}\left(\log_{2}\left(o^{2}\right)\right)\right)}{8}\right)\left\{o>1\right\}

\left(o_{seq},\frac{B_{house}\left(o_{seq}\right)}{B_{naive}\left(o_{seq}\right)}\right)
```