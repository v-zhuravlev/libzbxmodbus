FROM gitpod/workspace-full
                    
USER gitpod

RUN sudo apt-get -q update && \
    sudo apt-get install -yq autoconf automake libtool pkg-config bear && \
    sudo rm -rf /var/lib/apt/lists/*
