# Build from Ubuntu image, should be able to be run from Windows and MacOsx using Docker Desktop
FROM ubuntu:latest

# Install Wine and other dependencies
RUN dpkg --add-architecture i386 && \
    apt-get update -y && \
    apt-get install -y wine64 wine32

# Setup environment variables (ignore, basically just to keep Wine happy)
ENV DEBIAN_FRONTEND=noninteractive
ENV WINEARCH=win64
ENV WINEPREFIX=/root/.wine64
ENV WINEDEBUG=-all

# Install code dependencies
RUN apt install pip -y
RUN pip install numpy
RUN pip install pandas
RUN apt-get install -y xvfb

# Set the working directory in the container
WORKDIR /Docker

# Copy the required files to the docker and make it runnable
RUN mkdir -p /usr/bin/
COPY entrypoint.sh /usr/bin/entrypoint.sh
RUN chmod +x /usr/bin/entrypoint.sh
COPY /linux_runner.py .
COPY /2023_block_coalescing_reference_implementation.exe .

# !!! Change these to the algorithm and test case you want to run (make sure you change the entry point file too)
COPY /cRLE.exe .
COPY /Inputs/the_combinatorial_one_42000000_14x10x12.csv .


# Run entrypoint
ENTRYPOINT ["/usr/bin/entrypoint.sh"]
