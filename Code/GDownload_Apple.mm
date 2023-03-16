#import "GDownload.h"
#ifdef __APPLE__
#import <Foundation/Foundation.h>



struct GDownload::Private {
	NSURLSessionDataTask* task;
};



GDownload::GDownload ()
: _data(new Private)
{
}

GDownload::GDownload (const std::string& url, const std::string& method, const std::vector<int8_t>& body)
: _data(new Private)
{
	Start(url, method, body);
}

GDownload::~GDownload () {
}

bool GDownload::Start (const std::string& url, const std::string& method, const std::vector<int8_t>& body) {
	NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]]];
	request.HTTPMethod = [NSString stringWithUTF8String:method.c_str()];
	[request addValue:@"" forHTTPHeaderField:@"Accept-Encoding"];
	if(!body.empty())
		request.HTTPBody = [NSData dataWithBytes:body.data() length:body.size()];
	NSURLSessionConfiguration* config = [NSURLSessionConfiguration defaultSessionConfiguration];
	NSURLSession* session = [NSURLSession sessionWithConfiguration:config];
	_data->task = [session dataTaskWithRequest:request completionHandler:^(NSData* data, NSURLResponse* response, NSError* error) {
		if(!error) {
			const char *bytes = (const char*)[data bytes];
			this->data = std::vector<int8_t>(bytes, bytes + [data length]);
		}
	}];
	[_data->task resume];
	return true;
}

bool GDownload::IsDownloading () {
	return [_data->task state] == NSURLSessionTaskStateRunning;
}

float GDownload::GetProgress () {
	
	if([_data->task state] == NSURLSessionTaskStateCompleted)
		return 1.0f;
	
	if([_data->task countOfBytesExpectedToReceive] <= 0)
		return 0.0f;
	
	return (float)[_data->task countOfBytesReceived] / (float)[_data->task countOfBytesExpectedToReceive];
}



#endif // __APPLE__
