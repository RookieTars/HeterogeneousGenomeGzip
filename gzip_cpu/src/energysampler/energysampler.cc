#include "energysampler.h"
#include <fstream>
#include <fcntl.h>
#include <unistd.h>

energySampler::energySampler()
{
    file_u = fopen("/sys/class/power_supply/battery/voltage_now", "r");
    file_u_in = fopen("/sys/class/power_supply/usb/voltage_now", "r");
    file_i = fopen("/sys/class/power_supply/battery/current_now", "r");
    file_i_in = fopen("/sys/class/power_supply/usb/input_current_now", "r");
    if (file_u == NULL)
    {
        std::cout << "error u" << std::endl;
    }
    if (file_u_in == NULL)
    {
        std::cout << "error u" << std::endl;
    }
    if (file_i == NULL)
    {
        std::cout << "error i" << std::endl;
    }
    if (file_i_in == NULL)
    {
        std::cout << "error i in" << std::endl;
    }
}

energySampler::~energySampler()
{
    fclose(file_u);
    fclose(file_u_in);
    fclose(file_i);
    fclose(file_i_in);
}

void energySampler::start()
{
    running = true;
    runner = std::thread(
        [&]
        {
        long u,u_in,i,i_in;
        while(running){
            fseek(file_u, 0, SEEK_SET);
            fscanf(file_u, "%ld", &u);
            U.push_back(u);
            fseek(file_u_in, 0, SEEK_SET);
            fscanf(file_u_in, "%ld", &u_in);
            U_IN.push_back(u_in);
            fseek(file_i, 0, SEEK_SET);
            fscanf(file_i, "%ld", &i);
            I.push_back(i);
            fseek(file_i_in, 0, SEEK_SET);
            fscanf(file_i_in, "%ld", &i_in);
            I_IN.push_back(i_in);
            tp.push_back(getTimeNs());
            std::this_thread::sleep_for(std::chrono::nanoseconds(25000000));
        } });
    std::cout << getTimeNs() << std::endl;
}

void energySampler::end()
{
    running = false;
    if (runner.joinable())
    {
        runner.join();
    }
    else
    {
        std::cout << "energySampler didn't run normally" << std::endl;
    }
}

void energySampler::out(std::string outfilepath)
{
    std::ofstream fout(outfilepath);
    
    fout << "U,I,U_IN,I_IN,time,p" << std::endl;
    for (int i = 0; i < U.size(); i++)
    {
        long p; 
        p=abs(U[i]*I[i])+abs(U_IN[i]*I_IN[i]);
        fout << U[i] << "," << I[i] << "," << U_IN[i] << "," << I_IN[i] << "," << tp[i] <<","<<p<< std::endl;
    }
    fout.close();
}
