# Code Infinity Proficiency Test number 2

## Manipulating arrays and file handling.

This application is developed using NodeJS and typescript. It also includes a `data_manager` written in C with a C++ wrapper for NodeJS. This was developed on a Linux distribution using a GNU toolchain, it is therefore advised to install an run this on Linux.

In order to build the C/C++ code you need to install the GNU Compiler Collection (GCC) toolchain as well as the `libsqlite3-dev` library. You can do so by running the following command:

```shell
sudo apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    python3 \
    libsqlite3-dev \
    git \
    pkg-config
```

Once the toolchain is installed you can continue building the app.

First you need to install it by running `yarn install` or `npm install`. This will install the node packages as well as build the C/C++ addon.

Then you can run the application by running `yarn dev` or `npm run dev` and then pointing your browser to `http://localhost:3000/`.
