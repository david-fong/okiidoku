#ifndef HPP_OKIIDOKU__PUZZLE__SOLVER__FIND_CACHE
#define HPP_OKIIDOKU__PUZZLE__SOLVER__FIND_CACHE

#include <okiidoku/o2_bit_arr.hpp>

namespace okiidoku::mono::detail::solver2 {

	template<Order O> requires(is_order_compiled(O))
	struct FindCacheForSubFishHouseType final {
	private:
		using T = Ints<O>;
		using o2i_t = int_ts::o2i_t<O>;
	public:
		struct HouseData final {
			struct Tag {
				int_ts::o2is_t<O> num_cands_cache;
				int_ts::o2xs_t<O> at;
			};
			O2BitArr<O> is_begin;
			std::array<Tag, T::O2> tags;
		}
	private:
		std::array<HouseData, T::O2> houses_;
	public:
		[[nodiscard, gnu::pure]]
		HouseData& operator[](const o2i_t house) noexcept { return houses_[house]; }
	};


	template<Order O> requires(is_order_compiled(O))
	struct FindCacheForSubsets final {
	private:
		using T = Ints<O>;
		using o2i_t = int_ts::o2i_t<O>;
		HouseTypeMap<FindCacheForSubFishHouseType> types_;
	public:
		[[nodiscard, gnu::pure]]
		auto& house(const HouseType type, const o2i_t house) noexcept { return types_.at(type)[house]; }
	};
	// TODO wait... note: if we have separate for each CellOrSym major POV, need to design a way to quickly sync them.


	template<Order O> requires(is_order_compiled(O))
	struct FindCacheForFish final {
	private:
		using T = Ints<O>;
		using o2i_t = int_ts::o2i_t<O>;
		LineTypeMap<FindCacheForSubFishHouseType> types_;
	public:
		[[nodiscard, gnu::pure]]
		auto& house(const LineType type, const o2i_t house) noexcept { return types_.at(type)[house]; }
	};
}
#endif