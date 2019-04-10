#include "util.h"
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <map>
#include "cubemap.h"
#include "harmonics.h"

using namespace std;

std::string CoefficientsString(const std::vector<Vec3>& coefs)
{
	ostringstream oss;
	for (const Vec3& c : coefs)
	{
		oss << c.r << "\t" << c.g << "\t" << c.b << std::endl;
	}
	return oss.str();
}

int main(int argc, char* argv[])
{
	int degree = 3;
	int samplenum = 1000000;
	// read arguments
	if (argc <3 || argc > 6)
	{
		cout << "Usage: ./sampler directory format [degree samplenum --write-rendered]" << endl;
		return 1;
	}

	string dir = argv[1];
	if (dir.back() != '/' && dir.back() != '\\')
		dir += '/';
	array<string, 6> faces = {"posx", "negx", "posy", "negy", "posz", "negz"};
	array<std::string, 6> img_files;
	string format = argv[2];
	for (int i = 0; i < 6; i++)
		img_files[i] = dir + faces[i] + "." + format;

	bool write_rendered = false;
	if (argc >= 4)
		degree = stoi(argv[3]);
	if(argc >= 5)
		samplenum = stoi(argv[4]);
	if (argc >= 6 && string(argv[5]) == "--write-rendered")
		write_rendered = true;

	// output directory
	string outdir = dir + "output-images/";
	if (write_rendered)
	{
		string mkdircmd = "mkdir " + outdir;
		replace(mkdircmd.begin(), mkdircmd.end(), '/', '\\');
		system(mkdircmd.c_str());
	}

	try {

		// sampling
		cout << "reading cubemap ..." << endl;
		Cubemap cubemap(img_files);
		if(write_rendered)
		{
			string expandfile = outdir + "expand." + format;
			cout << "write expand cubemap image: " << expandfile << endl;
			cv::Mat expand = cubemap.GenExpandImage();
			cv::imwrite(expandfile, expand * 255);
		}

		Harmonics harmonics(degree);
		{
			cout << "sampling ..." << endl;
			auto verticies = cubemap.RandomSample(samplenum);
			harmonics.Evaluate(verticies);
		}

		cout << "---------- coefficients ----------" << endl;
		auto coefs = harmonics.getCoefficients();
		string coefstr = CoefficientsString(coefs);
		cout << coefstr;
		cout << "----------------------------------" << endl;

		ofstream coeffile(dir + "coefficients.txt");
		if (coeffile)
		{
			coeffile << coefstr;
			cout << "written " << dir + "coefficients.txt" << endl;
		}
		else
			cout << "write coefficients.txt failed" << endl;


		if(write_rendered)
		{
			cout << "rendering ..." << endl;
			auto shimgs = harmonics.RenderCubemap(cubemap.Width(), cubemap.Height());

			for (int i = 0; i < 6; i++)
			{
				string outfile = outdir + "rendered_" + faces[i] + "." + format;
				cout << "write rendered images: " << outfile << endl;
				cv::imwrite(outfile, shimgs[i] * 255);
			}
			Cubemap shcubemap(shimgs);

			string shexpandfile = outdir + "rendered_expand." + format;
			cout << "write rendered expand cubemap image: " << shexpandfile << endl;
			cv::Mat shexpand = shcubemap.GenExpandImage();
			cv::imwrite(shexpandfile, shexpand * 255);
		}

		cout << "done !" << endl;
	}
	catch (std::exception e)
	{
		cout << "***** AN ERROR OCCURRED *****" << endl;
		cout << e.what() << endl;
		return 1;
	}
	return 0;
}