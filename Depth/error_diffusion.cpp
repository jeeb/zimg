#include <algorithm>
#include <cstddef>
#include "Common/except.h"
#include "Common/plane.h"
#include "dither.h"
#include "error_diffusion.h"
#include "quantize.h"

namespace zimg {;
namespace depth {;

namespace {;

class ErrorDiffusionC : public DitherConvert {
	template <class T, class U, class Unpack, class Quant, class Dequant>
	void dither(const ImagePlane<const T> &src, const ImagePlane<U> &dst, float *tmp, Unpack unpack, Quant quant, Dequant dequant) const
	{		
		int width = src.width();
		int height = src.height();

		float *line0 = tmp + 1;
		float *line1 = line0 + width + 2;
		bool even = false;

		std::fill_n(tmp, ((size_t)width + 2) * 2, static_cast<T>(0));

		for (ptrdiff_t i = 0; i < height; ++i) {
			float *prev_line = even ? line1 : line0;
			float *curr_line = even ? line0 : line1;

			for (ptrdiff_t j = 0; j < width; ++j) {
				float x = unpack(src[i][j]);
				float err = 0;

				err += curr_line[j - 1] * (7.0f / 16.0f);
				err += prev_line[j + 1] * (3.0f / 16.0f);
				err += prev_line[j + 0] * (5.0f / 16.0f);
				err += prev_line[j - 1] * (1.0f / 16.0f);

				x += err;

				U q = quant(x);
				float y = dequant(q);

				dst[i][j] = q;
				curr_line[j] = x - y;
			}

			even = !even;
		}
	}
public:
	void byte_to_byte(const ImagePlane<const uint8_t> &src, const ImagePlane<uint8_t> &dst, float *tmp) const override
	{
		dither(src, dst, tmp,
		       make_integer_to_float<uint8_t>(src.format()),
		       make_float_to_integer<uint8_t>(dst.format()),
		       make_integer_to_float<uint8_t>(dst.format()));
	}

	void byte_to_word(const ImagePlane<const uint8_t> &src, const ImagePlane<uint16_t> &dst, float *tmp) const override
	{
		dither(src, dst, tmp,
		       make_integer_to_float<uint8_t>(src.format()),
		       make_float_to_integer<uint16_t>(dst.format()),
		       make_integer_to_float<uint16_t>(dst.format()));
	}

	void word_to_byte(const ImagePlane<const uint16_t> &src, const ImagePlane<uint8_t> &dst, float *tmp) const override
	{
		dither(src, dst, tmp,
		       make_integer_to_float<uint16_t>(src.format()),
		       make_float_to_integer<uint8_t>(dst.format()),
		       make_integer_to_float<uint8_t>(dst.format()));
	}

	void word_to_word(const ImagePlane<const uint16_t> &src, const ImagePlane<uint16_t> &dst, float *tmp) const override
	{
		dither(src, dst, tmp,
		       make_integer_to_float<uint16_t>(src.format()),
		       make_float_to_integer<uint16_t>(dst.format()),
		       make_integer_to_float<uint16_t>(dst.format()));
	}

	void half_to_byte(const ImagePlane<const uint16_t> &src, const ImagePlane<uint8_t> &dst, float *tmp) const override
	{
		dither(src, dst, tmp,
		       depth::half_to_float,
		       make_float_to_integer<uint8_t>(dst.format()),
		       make_integer_to_float<uint8_t>(dst.format()));
	}

	void half_to_word(const ImagePlane<const uint16_t> &src, const ImagePlane<uint16_t> &dst, float *tmp) const override
	{
		dither(src, dst, tmp,
		       depth::half_to_float,
		       make_float_to_integer<uint16_t>(dst.format()),
		       make_integer_to_float<uint16_t>(dst.format()));
	}

	void float_to_byte(const ImagePlane<const float> &src, const ImagePlane<uint8_t> &dst, float *tmp) const override
	{
		dither(src, dst, tmp,
		       identity<float>,
		       make_float_to_integer<uint8_t>(dst.format()),
		       make_integer_to_float<uint8_t>(dst.format()));
	}

	void float_to_word(const ImagePlane<const float> &src, const ImagePlane<uint16_t> &dst, float *tmp) const override
	{
		dither(src, dst, tmp,
		       identity<float>,
		       make_float_to_integer<uint16_t>(dst.format()),
		       make_integer_to_float<uint16_t>(dst.format()));
	}
};

} // namespace


DitherConvert *create_error_diffusion(CPUClass cpu)
{
	return new ErrorDiffusionC{};
}

} // namespace depth
} // namespace zimg
