#include "../../StdAfx.h"

#include "image_producer.h"
#include "image_loader.h"

#include "../../processor/frame_processor_device.h"
#include "../../processor/draw_frame.h"
#include "../../format/video_format.h"

#include <boost/assign.hpp>

#include <algorithm>

using namespace boost::assign;

namespace caspar { namespace core { namespace image{

struct image_producer : public frame_producer
{	
	std::shared_ptr<frame_processor_device> frame_processor_;
	std::wstring filename_;
	safe_ptr<draw_frame> frame_;

	image_producer(image_producer&& other) 
		: frame_processor_(std::move(other.frame_processor_))
		, filename_(std::move(other.filename_))
		, frame_(draw_frame::empty()){}

	image_producer(const std::wstring& filename) 
		: filename_(filename), frame_(draw_frame::empty())	{}
	
	virtual safe_ptr<draw_frame> receive(){return frame_;}

	virtual void initialize(const safe_ptr<frame_processor_device>& frame_processor)
	{
		frame_processor_ = frame_processor;
		auto bitmap = load_image(filename_);
		FreeImage_FlipVertical(bitmap.get());
		auto frame = frame_processor->create_frame(FreeImage_GetWidth(bitmap.get()), FreeImage_GetHeight(bitmap.get()));
		std::copy_n(FreeImage_GetBits(bitmap.get()), frame->image_data().size(), frame->image_data().begin());
		frame_ = std::move(frame);
	}

	virtual std::wstring print() const
	{
		return L"image_producer. filename: " + filename_;
	}
};

safe_ptr<frame_producer> create_image_producer(const  std::vector<std::wstring>& params)
{
	static const std::vector<std::wstring> extensions = list_of(L"png")(L"tga")(L"bmp")(L"jpg")(L"jpeg");
	std::wstring filename = params[0];
	
	auto ext = std::find_if(extensions.begin(), extensions.end(), [&](const std::wstring& ex) -> bool
		{					
			return boost::filesystem::is_regular_file(boost::filesystem::wpath(filename).replace_extension(ex));
		});

	if(ext == extensions.end())
		return frame_producer::empty();

	return make_safe<image_producer>(filename + L"." + *ext);
}

}}}