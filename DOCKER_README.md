#Docker Guide
#1. Install Docker Desktop
https://docs.docker.com/desktop/
Windows, Mac and Linux installs, follow guide.

#2. Change the 'Dockerfile'
Find the '!!!' comment in the dockerfile, here you can specify where the algorithm and input that you want to run are
Docker has to be run from the base folder of the project, so create the path from there

#3. Change the 'Entrypoint.sh' file
Leave lines 1-4
Change line 5 to be the command you want to run
Should be in the form
'python3 linux_runner.py (algo.exe) (input.csv) -v'
-v option is to get more information from the runner, you can turn it off if you think it might increase speed

#4. Build and run the Docker
On Mac or Linux, open terminal to BLOCK6
On Windows, open cmd.exe to BLOCK6

Run: 'docker build -t competition_runner .' to build (can change name from competition_runner if you want)
Run: 'docker run --cpus=2 --memory=4096m competition_runner' to run

Use 'docker run --help' to see all the specs settings:

https://www.amazonaws.cn/en/ec2/instance-types/ 
T2 Instances either run this, https://www.cpubenchmark.net/cpu.php?cpu=Intel+Xeon+E5-2676+v3+%40+2.40GHz&id=2643 (worse) 
or this, https://www.cpubenchmark.net/cpu.php?cpu=Intel+Xeon+E5-2686+v4+%40+2.30GHz&id=2870 (better)

https://aws.amazon.com/ec2/instance-types/t2/
T2.medium, which is what the competition runs, has 2 vThreads and 4 GBs of RAM, hence my settings for the runner.
