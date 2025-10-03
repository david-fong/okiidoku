// SPDX-FileCopyrightText: 2020 David Fong
// SPDX-License-Identifier: AGPL-3.0-or-later
#ifndef HPP_OKIIDOKU_PRINT_2D_EMOJI
#define HPP_OKIIDOKU_PRINT_2D_EMOJI

#include <tuple>
#include <array>
#include <string_view>
#include <cstddef> // size_t

namespace okiidoku::emoji {

	template<std::size_t size>
	struct Set {
		std::string_view name;
		std::array<std::string_view, size> entries;
	};

	// TODO.low annoyingly, when printing on various terminal emulators, some
	// emojis vary in width. I've added spaces where required on gnome terminal,
	// but windows terminal is different.
	inline constexpr std::tuple sets {
		#define E std::to_array<std::string_view>
		// NOLINTBEGIN(*-designated-initializers)
		Set{"fruit",       E({"🍉","🍄","🍓","🍅","🌶 ","🍎","🍑","🍊","🥕","🥭","🍍","🍋","🍌","🌽","🥑","🍐","🥝","🍏","🥬","🍈","🫐","🧄","🍆","🍇","🌰"}) },
		Set{"reptile",     E({"🐸","🦎","🐍","🐢","🐊","🐉","🐲","🦕","🦖"}) },
		Set{"marine",      E({"🦦","🦭","🦈","🐬","🐋","🐟","🐠","🦑","🐙","🦀","🦞","🐚"}) },
		Set{"sweets",      E({"🍪","🍫","🍬","🍭","🍰","🍮","🍡","🥠","🍦","🍯","🍩"}) },
		Set{"vehicle",     E({"🚗","🚑","🚒","🚓","🚈","🚠","🛒","🚀","🛸"}) },
		Set{"plant",       E({"🌱","🌲","🌳","🌴","🌵","🌹","🌷","🌸","🌺","🏵 ","🌼","🍀","🍁","🍂","🍃"}) },
		Set{"animal_body", E({"🐁","🦔","🐇","🦨","🐈","🐕","🐖","🐄","🐑","🐐","🐎","🐒","🦍","🦥","🦒","🦏","🐘","🐫"}) },
		Set{"animal_head", E({"🐰","🐱","🐶","🐔","🐴","🐮","🐷","🦝","🐺","🦁","🐯","🐵","🐻","🐼","🐨","🦄","🦓"}) },
		Set{"bird",        E({"🐣","🐤","🐦","🕊 ","🦩","🦅","🦆","🦉","🦚","🦜","🐧","🦢","🐓"}) },
		Set{"insect",      E({"🐜","🕷 ","🪰","🐞","🦋","🐝","🐌","🦗"}) },
		Set{"food",        E({"🧀","🥨","🥐","🍞","🥪","🍔","🍟","🍕","🌮","🥗","🍚","🍜","🍥","🍤","🍣","🍙","🍘"}) },
		Set{"drink",       E({"🫖","🍺","☕","🍵","🧋","🧃","🧂","🧊"}) },
		Set{"body",        E({"🧠","👀","👁 ","👂","👃","🦷","👄","💋","👅","✋","👆","👇","👈","👉","👋","👌","👍","👎"}) },
		Set{"clothing",    E({"👑","🎀","🧢","👒","🎓","🎩","👓","🤿","💄","🧣","👕","👚","👗","👘","🧤","👛","👜","🎒","🧦","👟","👠","👢"}) },
		Set{"heart",       E({"💖️","❤️ ","🧡","💛","💚","💙","💜","🖤","💞","🫀"}) },
		Set{"cartoon",     E({"💤","💢","💎","💣","💀","👾","💥","💦","💨","💬","💭","💩"}) },
		Set{"smiley",      E({"😀","😍","😎","😛","😐","😗","😮","🤯","😡","😨","🙃"}) },
		Set{"sport",       E({"⚽","⚾","🏐","🏀","🏈","🥊","🎾","🏓","🥌","🎳","🎱","🎯","🀄","🃏","🎲","🧵","🧶","🎮","🔫","🏆"}) },
		Set{"occasion",    E({"🎂","🎁","🎃","🎄","🎈","🎉","🎊","🎏"}) },
		Set{"element",     E({"✨","⭐","🌈","💧","🔥","⚡","☃️ ","☔","⚓","⛺","⛽","🌊","🌋","🎨","🎪","🏯","🏰","💈","💺","🗻"}) },
		Set{"sign",        E({"🚨","🛑","⚠️ ","⛔","♻️ ","✅","❌","💯"}) },
		Set{"household",   E({"🪑","🚽","🪠","🧺","🧻","🪣","🧼","🧽","🧯"}) }, // 🛋
		Set{"media",       E({"🎬","🎵","🎷","🎺","🎻","🎸","🎹","🥁"}) },
		Set{"office",      E({"⏳","🩺","🩹","🧲","🧭","💰","💡","🔐","🔑","🔔","📢","📌","📎","📂","🗑️ ","💾","💽","💻","📺","📷","📼","📨","📬","📦"}) },
		Set{"circle",      E({"🔮","🗿","🚬","🔴","🟠️","🟡️","🟢️","🔵","🟣️","🟤️","⚫️","⚪️"}) },
		// NOLINTEND(*-designated-initializers)
		#undef E
	};
}
#endif