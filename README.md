# Code Infinity Proficiency Test number 2

## Manipulating arrays and file handling.

This application is developed using NodeJS and typescript. It also includes a `data_manager` written in C with a C++ wrapper for NodeJS. This was developed on a Debian Linux distribution using a GCC toolchain, it is therefore advised to install an run this on a Debian/Ubuntu Linux distribution.

### Setting up the build environment

In order to build the C/C++ code you need to install the GNU Compiler Collection (GCC) toolchain as well as the `libsqlite3-dev` library. You can do so by running the following command:

```shell
sudo apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    python3 \
    libsqlite3-dev \
    git \
    pkg-config
```

To install for other GNU/Linux distributions you can search the internet for instructions on how to do that.

### Installing the application

Once the toolchain is installed you can continue building the app.

First you need to install it by running `yarn install` or `npm install`. This will install the node packages as well as build the C/C++ addon.

### Running the app

Then you can run the application by running `yarn dev` or `npm run dev` and then pointing your browser to `http://localhost:3000/`.

### Notes

The application presents two forms to the user each performing a specific function as specified under Test 2 in the [Code Infinity Proficiency Test](docs/TEST2.md). The functions of the two forms are as follows:

#### Form A (left)

This form allows the user to generate a CSV file called `output.csv` containing any number of records of randomized data. The shape of this data is described in [Code Infinity Proficiency Test](docs/TEST2.md).

#### Form B (right)

This form allows the user to upload the CSV file generated on `Form A`. It is then used to create a `SQLite3` database and populate a database table called `csv_import`.

It is important to note that the `output.csv` file that is generated is saved in the project root together with the `SQLIte3` database file - `data.db`. When uploading the file using `Form B`, make sure that you browse to thís location to find the generated file.

Enjoy!
