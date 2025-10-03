
#include "rtweekend.h"

#include "Color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "IExecutionEvent.h"
#include "ThreadPool.h"

#include <iostream>
#include <mutex>

#include "PNGImage.h"

double hit_sphere(const point3& center, double radius, const ray& r) {
	vec3 oc = r.origin() - center;
	auto a = r.direction().length_squared();
	auto half_b = dot(oc, r.direction());
	auto c = oc.length_squared() - radius * radius;
	auto discriminant = half_b * half_b - a * c;
	if (discriminant < 0) {
		return -1.0;
	}
	else {
		return (-half_b - sqrt(discriminant)) / a;
	}
}
color ray_color(const ray& r, const hittable& world, int depth) {
	hit_record rec;

	// Recursive base case
	if (depth <= 0)	return color(0, 0, 0);

	if (world.hit(r, 0.001, infinity, rec)) {
		ray scattered;
		color attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth - 1);
		return color(0, 0, 0);
	}
	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

hittable_list book_scene() {
	hittable_list world;
	auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));
	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
			if ((center - point3(4, 0.2, 0)).length() > 0.9) {
				shared_ptr<material> sphere_material;
				if (choose_mat < 0.8) {
					// diffuse
					auto albedo = random() * random();
					sphere_material = make_shared<lambertian>(albedo);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95) {
					// metal
					auto albedo = random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else {
					// glass
					sphere_material = make_shared<dielectric>(1.5);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}
	auto material1 = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));
	auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));
	auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));
	return world;
}

hittable_list random_scene() {
	hittable_list world;
	auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

	double xPosRange = 7.0;
	double zPosRange = 4.0;

	for (int i = 0; i < 500; i++) {
		point3 center;
		double choose_mat = random_double();

		do {
			double xOffset = (random_double() * 2.0) - 1.0;
			double zOffset = (random_double() * 2.0) - 1.0;
			double xPos = xOffset * xPosRange;
			double zPos = zOffset * zPosRange;

			center = point3(xPos, 0.2, zPos);
		} while ((center - point3(4, 0.2, 0)).length() < 0.9);

		shared_ptr<material> sphere_material;
		if (choose_mat < 0.8) {
			// diffuse
			auto albedo = random() * random();
			sphere_material = make_shared<lambertian>(albedo);
			world.add(make_shared<sphere>(center, 0.2, sphere_material));
		}
		else if (choose_mat < 0.95) {
			// metal
			auto albedo = random(0.5, 1);
			auto fuzz = random_double(0, 0.5);
			sphere_material = make_shared<metal>(albedo, fuzz);
			world.add(make_shared<sphere>(center, 0.2, sphere_material));
		}
		else {
			// glass
			sphere_material = make_shared<dielectric>(1.5);
			world.add(make_shared<sphere>(center, 0.2, sphere_material));
		}
	}

	auto material1 = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));
	auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));
	auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));
	auto material4 = make_shared<lambertian>(color(0.5, 0.2, 0.2));
	world.add(make_shared<sphere>(point3(-6, 1, 1), 1.0, material4));
	return world;
}

class IImageWriter : public IExecutionEvent {
public:
	IImageWriter(camera* cam, hittable* world, int image_width, int image_height, int samples_per_pixel, int max_depth) :
		cam(cam), world(world), image_width(image_width), image_height(image_height), samples_per_pixel(samples_per_pixel), max_depth(max_depth) {};

	virtual void Run() = 0;
	virtual void WriteHeader() = 0;
	virtual void WritePixel(int x, int y) = 0;

protected:
	camera* cam;
	hittable* world;
	const int image_width;
	const int image_height;
	const int samples_per_pixel;
	const int max_depth;
};

class PPMNonThreadedWriter : public IImageWriter {
public:
	PPMNonThreadedWriter(camera* cam, hittable* world, int image_width, int image_height, int samples_per_pixel, int max_depth) :
		IImageWriter(cam, world, image_width, image_height, samples_per_pixel, max_depth) {
	}

	void Run() override {
		WriteHeader();
		for (int j = image_height - 1; j >= 0; --j) {
			std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
			for (int i = 0; i < image_width; ++i) {
				WritePixel(i, j);
			}
		}

		std::cerr << "\nDone.\n";
	}
	void WriteHeader() override {
		std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
	}
	void WritePixel(int x, int y) override {
		color pixel_color(0, 0, 0);
		for (int s = 0; s < samples_per_pixel; ++s) {
			auto u = (x + random_double()) / (image_width - 1);
			auto v = (y + random_double()) / (image_height - 1);
			ray r = cam->get_ray(u, v);
			pixel_color += ray_color(r, *world, max_depth);
		}
		write_color(std::cout, pixel_color, samples_per_pixel);
	}
	void OnFinishedExecution() override {
		// Not used in non-threaded version
	}
};

class PNGNonThreadedWriter : public IImageWriter {
public:
	PNGNonThreadedWriter(std::string filename, camera* cam, hittable* world, int image_width, int image_height, int samples_per_pixel, int max_depth) :
		IImageWriter(cam, world, image_width, image_height, samples_per_pixel, max_depth),
		filename(filename)
	{
		image = new PNGImage(image_width, image_height);
	}

	void Run() override {
		for (int j = image_height - 1; j >= 0; --j) {
			std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
			for (int i = 0; i < image_width; ++i) {
				WritePixel(i, j);
			}
		}

		std::cerr << "\nExporting...\n";
		ExportPNG();
		std::cerr << "\nDone.\n";
	}
	void WriteHeader() override {
		// Not used for PNG, opencv handles png writing
	}
	void ExportPNG() {
		image->SaveImage(filename);
	}
	void WritePixel(int x, int y) override {
		color pixel_color(0, 0, 0);
		for (int s = 0; s < samples_per_pixel; ++s) {
			auto u = (x + random_double()) / (image_width - 1);
			auto v = (y + random_double()) / (image_height - 1);
			ray r = cam->get_ray(u, v);
			pixel_color += ray_color(r, *world, max_depth);
		}
		image->SetPixel(x, y, pixel_color.x(), pixel_color.y(), pixel_color.z(), samples_per_pixel);
	}
	void OnFinishedExecution() override {
		// Not used in non-threaded version
	}

private:
	PNGImage* image = nullptr;
	std::string filename = nullptr;
};

class PPMWriteRowAction : public IWorkerAction {

public:
	PPMWriteRowAction(IImageWriter* writer, int img_width, int y) : ppmWriter(writer), image_width(img_width), y(y) {};

	virtual void OnStartTask() override {
		if (ppmWriter == nullptr) return;

		//std::cerr << "\rScanlines remaining (started one): " << y << ' ' << std::endl;

		for (int i = 0; i < image_width; ++i) {
			ppmWriter->WritePixel(i, y);
		}

		//std::cerr << "\rOne task completed!" << std::endl;

		ppmWriter->OnFinishedExecution();
	}

private:
	IImageWriter* ppmWriter;
	int image_width;
	int y;
};

class PPMWriteBlockAction : public IWorkerAction {
public:
	PPMWriteBlockAction(IImageWriter* writer, int startX, int startY, int blockWidth, int blockHeight) :
		ppmWriter(writer), startX(startX), startY(startY), blockWidth(blockWidth), blockHeight(blockHeight) {
	};

	virtual void OnStartTask() override {
		if (ppmWriter == nullptr) return;
		//std::string str = "\nWrite Block: x(" + std::to_string(startX) + ", " + std::to_string(startX + blockWidth) + "), y(" + std::to_string(startY) + ", " + std::to_string(startY + blockHeight) + ")";
		//std::cerr << str;
		for (int y = startY; y < startY + blockHeight; ++y) {
			for (int x = startX; x < startX + blockWidth; ++x) {
				ppmWriter->WritePixel(x, y);
			}
		}
		ppmWriter->OnFinishedExecution();
	}
private:
	IImageWriter* ppmWriter;
	int startX;
	int startY;
	int blockWidth;
	int blockHeight;
};

class PPMThreadedWriter : public IImageWriter {
public:
	PPMThreadedWriter(camera* cam, hittable* world, int image_width, int image_height, int samples_per_pixel, int max_depth, int maxThreadCount) :
		IImageWriter(cam, world, image_width, image_height, samples_per_pixel, max_depth), 
		threadPool(maxThreadCount),
		pixelData(image_width * image_height),
		block_height(1), block_width(image_width)
	{
		threadPool.StartScheduling();
	}
	PPMThreadedWriter(camera* cam, hittable* world, int image_width, int image_height, int samples_per_pixel, int max_depth, int maxThreadCount, int block_width, int block_height) :
		IImageWriter(cam, world, image_width, image_height, samples_per_pixel, max_depth),
		threadPool(maxThreadCount),
		pixelData(image_width* image_height),
		block_height(block_height), block_width(block_width)
	{
		threadPool.StartScheduling();
	}

	void Run() override {

		CreateBlockScans(block_width, block_height);

		std::cerr << "\rScans remaining: " << scan_total - completed_scans << ' ' << std::flush;

		while (!isFinished) {
			// Wait for rendering to complete
		}

		// Actually output to the cout
		std::cerr << "\nExporting...\n";
		WriteHeader();
		for (int i = 0; i < pixelData.size(); i++) {
			std::cout << pixelData[i] << std::endl;
		}

		std::cerr << "\nDone.\n";
	}

	/*void CreateRowScans() {
		scan_total = image_height;

		for (int j = image_height - 1; j >= 0; --j) {
			PPMWriteRowAction* action = new PPMWriteRowAction(this, image_width, j);
			threadPool.ScheduleTask(action);
		}

	}*/

	void CreateBlockScans(int blockX, int blockY) {
		int xBlocks = (image_width + blockX - 1) / blockX;
		int xOvershoot = image_width % blockX;

		int yBlocks = (image_height + blockY - 1) / blockY;
		int yOvershoot = image_height % blockY;

		int totalBlocks = xBlocks * yBlocks;
		scan_total = totalBlocks;

		for (int i = 0; i < yBlocks; i++) {
			for (int j = 0; j < xBlocks; j++) {
				int width = blockX;
				int height = blockY;

				if (i == yBlocks - 1 && yOvershoot != 0) height = yOvershoot;
				if (j == xBlocks - 1 && xOvershoot != 0) width = xOvershoot;
				
				PPMWriteBlockAction* action = new PPMWriteBlockAction(this, j * blockX, i * blockY, width, height);
				threadPool.ScheduleTask(action);
			}
		}

	}

	void WriteHeader() override {
		std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
	}
	void WritePixel(int x, int y) override {
		//std::cerr << "\rWriting Pixel: " << x << "," << y << ' ' << std::flush;
		color pixel_color(0, 0, 0);
		for (int s = 0; s < samples_per_pixel; ++s) {
			auto u = (x + threadsafe_random_double()) / (image_width - 1);
			auto v = (y + threadsafe_random_double()) / (image_height - 1);
			ray r = cam->threadsafe_get_ray(u, v);
			pixel_color += ray_color(r, *world, max_depth);
		}

		std::string pixel = get_color_string(pixel_color, samples_per_pixel);
		
		//std::lock_guard<std::mutex> guard(pixelDataMtx);
		pixelData[((image_height - y - 1) * image_width) + x] = pixel;
		
	}
	void OnFinishedExecution() override {
		completed_scans = completed_scans + 1;
		std::lock_guard<std::mutex> guard(cerrMtx);
		std::cerr << "\rScans remaining: " << scan_total - completed_scans << ' ' << std::flush;
		if (completed_scans >= scan_total) {
			isFinished = true;
			threadPool.StopScheduling();
		}

	}

	bool isFinished = false;

private:
	ThreadPool threadPool;
	int completed_scans = 0;
	int scan_total = 0;
	
	int block_width;
	int block_height;
	
	std::mutex pixelDataMtx;
	std::mutex cerrMtx;

	std::vector<std::string> pixelData;
};

class PNGThreadedWriter : public IImageWriter {
public:
	PNGThreadedWriter(std::string filename, camera* cam, hittable* world, int image_width, int image_height, int samples_per_pixel, int max_depth, int maxThreadCount) :
		IImageWriter(cam, world, image_width, image_height, samples_per_pixel, max_depth),
		threadPool(maxThreadCount),
		filename(filename),
		block_height(1), block_width(image_width)
	{
		image = new PNGImage(image_width, image_height);
		threadPool.StartScheduling();
	}
	PNGThreadedWriter(std::string filename, camera* cam, hittable* world, int image_width, int image_height, int samples_per_pixel, int max_depth, int maxThreadCount, int block_width, int block_height) :
		IImageWriter(cam, world, image_width, image_height, samples_per_pixel, max_depth),
		threadPool(maxThreadCount),
		filename(filename),
		block_height(block_height), block_width(block_width)
	{
		image = new PNGImage(image_width, image_height);
		threadPool.StartScheduling();
	}

	void Run() override {

		CreateBlockScans(block_width, block_height);

		std::cerr << "\rScans remaining: " << scan_total - completed_scans << ' ' << std::flush;

		while (!isFinished) {
			// Wait for rendering to complete
		}

		// Actually output to the cout
		std::cerr << "\nExporting...\n";
		ExportPNG();
		std::cerr << "\nDone.\n";
	}

	void CreateBlockScans(int blockX, int blockY) {
		int xBlocks = (image_width + blockX - 1) / blockX;
		int xOvershoot = image_width % blockX;

		int yBlocks = (image_height + blockY - 1) / blockY;
		int yOvershoot = image_height % blockY;

		int totalBlocks = xBlocks * yBlocks;
		scan_total = totalBlocks;

		for (int i = 0; i < yBlocks; i++) {
			for (int j = 0; j < xBlocks; j++) {
				int width = blockX;
				int height = blockY;

				if (i == yBlocks - 1 && yOvershoot != 0) height = yOvershoot;
				if (j == xBlocks - 1 && xOvershoot != 0) width = xOvershoot;

				PPMWriteBlockAction* action = new PPMWriteBlockAction(this, j * blockX, i * blockY, width, height);
				threadPool.ScheduleTask(action);
			}
		}

	}

	void WriteHeader() override {
		// Not used in PNG, let opencv handle this
	}
	void WritePixel(int x, int y) override {
		//std::cerr << "\rWriting Pixel: " << x << "," << y << ' ' << std::flush;
		color pixel_color(0, 0, 0);
		for (int s = 0; s < samples_per_pixel; ++s) {
			auto u = (x + threadsafe_random_double()) / (image_width - 1);
			auto v = (y + threadsafe_random_double()) / (image_height - 1);
			ray r = cam->threadsafe_get_ray(u, v);
			pixel_color += ray_color(r, *world, max_depth);
		}

		//std::lock_guard<std::mutex> guard(pixelDataMtx);
		image->SetPixel(x, y, pixel_color.x(), pixel_color.y(), pixel_color.z(), samples_per_pixel);
	}
	void ExportPNG() {
		image->SaveImage(filename);
	}
	void OnFinishedExecution() override {
		completed_scans = completed_scans + 1;
		std::lock_guard<std::mutex> guard(cerrMtx);
		std::cerr << "\rScans remaining: " << scan_total - completed_scans << ' ' << std::flush;
		if (completed_scans >= scan_total) {
			isFinished = true;
			threadPool.StopScheduling();
		}

	}

	bool isFinished = false;

private:
	ThreadPool threadPool;
	int completed_scans = 0;
	int scan_total = 0;

	int block_width;
	int block_height;

	std::mutex pixelDataMtx;
	std::mutex cerrMtx;

	PNGImage* image = nullptr;
	std::string filename = nullptr;

};



int main()
{
	// Image
	const auto aspect_ratio = 3.0 / 2.0;
	const int image_width = 1200;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 20;
	const int max_depth = 8;

	// World
	auto world = random_scene();

	// Camera
	point3 lookfrom(13, 2, 3);
	point3 lookat(0, 0, 0);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = 10.0;
	auto aperture = 0.1;

	camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

	// Render
	//PPMNonThreadedWriter imgWriter(&cam, &world, image_width, image_height, samples_per_pixel, max_depth);
	//PPMThreadedWriter imgWriter(&cam, &world, image_width, image_height, samples_per_pixel, max_depth, 40, 20, 20);
	//PNGNonThreadedWriter imgWriter("Single3x2.png", &cam, &world, image_width, image_height, samples_per_pixel, max_depth);
	PNGThreadedWriter imgWriter("ThreadedRandom40x20x8x20x20.png", &cam, &world, image_width, image_height, samples_per_pixel, max_depth, 40, 20, 20);

	imgWriter.Run();

	std::cerr << "Exiting Program.\n";

	return 0;
}
