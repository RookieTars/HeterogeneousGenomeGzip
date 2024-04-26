#include"energysampler/energysampler.h"
#include"MyGZip.h"

int main()
{
	int input = 0;
	energySampler e;
	

	string fileName;
	MyGZip mz;
	
	cout << "Enter the name of the compressed file path:";
	cin >> fileName;
	//e.start();
	mz.Deflate(fileName);
	//e.end();
    //e.out("/data/local/tmp/energySamplerResult-cpu.csv");
	return 0;
}