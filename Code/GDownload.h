#ifndef G_DOWNLOAD_H_
#define G_DOWNLOAD_H_

#import <vector>
#import <string>

// Use FetchAPI for this classes version for Web https://emscripten.org/docs/api_reference/fetch.html
// Use WebSocket API for UDP style https://emscripten.org/docs/porting/networking.html

class GDownload {
public:
	std::vector<int8_t> data;
	
	GDownload ();
	GDownload (const std::string& url, const std::string& method = "GET", const std::vector<int8_t>& body = {});
	~GDownload ();
	
	bool Start (const std::string& url, const std::string& method = "GET", const std::vector<int8_t>& body = {}); // Note, these are std::string and not std::string_view because they are converted to a c string in the functions
	bool IsDownloading ();
	float GetProgress ();
	
private:
	struct Private;
	std::unique_ptr<Private> _data;
};



#endif // G_DOWNLOAD_H_
