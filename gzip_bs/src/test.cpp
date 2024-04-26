#include "MyGZip.h"

int main(int argc, char **argv)
{
	int input = 0;
	// energySampler e;

	string fileName;
	MyGZip mz;

	// cout << "Enter the name of the compressed file path:";
	// cin >> fileName;
	fileName = "/data/local/tmp/AWRI796_ADVS01000000_cds.fsa";
	if (argc > 1)
	{
		fileName = (argv[argc - 1]);
	}

	// e.start();
	mz.Deflate(fileName);
	// e.end();
	// e.out("/data/local/tmp/energySamplerResult-cpu.csv");
	return 0;
}