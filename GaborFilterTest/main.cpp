 #include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>

int main(int argc, char** argv) {
	cv::Mat image = cv::imread("cat.jpg", 1);
	cv::Mat src;
	cv::cvtColor(image, src, CV_BGR2GRAY);
	cv::Mat src_f;
	//src.convertTo(src_f, CV_32F, 1.0/255, 0);
	src.convertTo(src_f, CV_32F, 1.0, 0);	// 255で割ると、フィルタかけた後、再度255かけないと、画像保存しても真っ黒になる

	int kernel_size = 21;	// 結果に影響しないらしい。sigmaに対して十分大きいサイズなら、大丈夫と思う。
	float sigma = 4.0f;//0.25f; // 大きくすると、ぼやける感じ。
	float lmbd = 10.0f; // 波長 (1なら、kernel_sizeで1周期ということ)。ストローク線の太さにあわせるべき？
	float gamma = 0.5f; // 波長のアスペクト比 (1じゃない方が、エッジを鋭く検出する感じ？)
	float psi = 0.0f / 180.0f * 3.14159265f; // 位相オフセット (0で良いだろう)

	for (int angle = 0; angle < 180; angle += 45) {
		float theta = (float)angle / 180.0f * 3.14159265f;
		
		cv::Mat kernel = cv::getGaborKernel(cv::Size(kernel_size, kernel_size), sigma, theta, lmbd, gamma, psi, CV_32F);
		kernel /= 1.0 * cv::sum(kernel)[0];
		//kernel /= 1.5 * cv::sum(kernel)[0];

		cv::Mat dest;
		cv::filter2D(src_f, dest, CV_32F, kernel);

		char filename1[256];
		sprintf(filename1, "result1_%d.jpg", angle);
		cv::imwrite(filename1, dest);

		cv::Mat mag;
        cv::pow(dest, 2.0, mag);

		char filename2[256];
		sprintf(filename2, "result2_%d.jpg", angle);
		cv::imwrite(filename2, mag);
	}
}
