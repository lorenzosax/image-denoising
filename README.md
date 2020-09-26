# Image Denoising
Implementing a parallel algorithm for image denoising with the Ising model using Markow Chain Monte Carlo.
Note that by images, black-white images (Images that consist of black and white pixels only) will be meant. And these black and white pixels will also be referred as -1 and 1, which is the representation of these images with text, provided in the project. We implemented 3 versions of this algorithm: sequential version, Pthreads version and MPI version.

## Sequential version
##### How to compile
```sh
$ gcc denoiser_sequential.c -o denoiser -lm
```
##### How to run
```sh
$ ./denoiser <input_file> <output_file> <beta> <pi>
```

## Pthreads version
##### How to compile
```sh
$ gcc denoiser_PTHREADS.c -o denoiser -lm -lpthread
```

##### How to run

```sh
$ ./denoiser <input_file> <output_file> <beta> <pi>
```

## MPI version
##### How to compile

```sh
$ mpicc denoiser.c -o denoiser -lm
```

##### How to run
Run the program using command (by default it will be run in grid mode):
```sh
$ mpiexec -np <nof_processors> ./denoiser <input_file> <output_file> <beta> <pi>
```
Optionally, it can run by the following command to run in row mode:
```sh
$ mpiexec -np <nof_processors> ./denoiser <input_file> <output_file> <beta> <pi> row
```

where:

- <nof_processors> Number of processors must satisfy a special condition, let’s say nof
proccessors is n, then, in row mode the width and height of the image must be
divisible by (n-1), in grid mode the width and height of the image must be divisible
by sqrt(n–1) and sqrt(n-1) must be an integer.
- <input_file> A text file which includes a n * n black-white image described with
1 and -1 values Each row must be terminated by a line ending and each column must
be separated by a space.
- <output_file> A file name to output denoised image, again as a n * n black-white
image described with 1 and -1 values where each row is terminated by a line ending
and each column is separated by a space.
- <beta> The beta value to be used as the beta value in the Metropolis-Hastings
Markov Chain Monte Carlo functions.
- <pi> The pi value to be used as the pi value in the Metropolis-Hastings Markov
Chain Monte Carlo functions.
- row That’s an optional parameter to run the program in row mode, by default the
program will be run in the grid mode. (An exact string “row” must be provided as the
argument)

Program outputs the denoised version of the initial image to the given
<output_file> path. It is the text version of a black-white image represented as a
grid of 1 and -1 values.


### Example of denoising image
Since the program works with text files containing -1 and 1, we need to convert the png image to text, for this we use a simple script python `scripts/image-to-text.py`. 
Another script is `scripts/make_noise.py`, this will add noise to our 'image' (text) and will be the input of parallel program.
Parallel program will produce an text file with denoised image and with `scripts/text_to_image.py` script will get a denoised image.

Original                    |  Noisy                   |  Denoised
:-------------------------:|:-------------------------: |:-------------------------:
![alt text](https://github.com/raffranco/Image-Denoising-MPI/blob/master/input-output/yinyang.png?raw=true)  |  ![alt text](https://github.com/raffranco/Image-Denoising-MPI/blob/master/input-output/yinyang_noisy.png?raw=true) | ![alt text](https://github.com/raffranco/Image-Denoising-MPI/blob/master/input-output/yinyang_output.png?raw=true)

## Authors

* **Raffaele Franco** - *Initial work* - [raffranco](https://github.com/raffranco)
* **Lorenzo Gagliani** - *Initial work* - [lorenzosax](https://github.com/lorenzosax)
