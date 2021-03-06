#include <SFML\Graphics.hpp>
#include <SFML\Window.hpp>
#include <SFML\System.hpp>
#include <iostream>


struct Vec4 {
	Vec4() :
		a(0), b(0), c(0), d(0) {

	}

	Vec4(int a, int b, int c, int d) :
		a(a), b(b), c(c), d(d) {

	}


	int a;
	int b;
	int c;
	int d;
};

class MyColor : public sf::Color {
public:

	MyColor() : sf::Color()	{

	}

	MyColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) :
		sf::Color(r, g, b, a) {

	}

	MyColor(const MyColor& rhs) {
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
		a = rhs.a;
	}

	MyColor(const sf::Color& rhs) {
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
		a = rhs.a;
	}

	MyColor& operator+=(const MyColor& rhs) {
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;
		a += rhs.a;

		return *this;
	}

	friend MyColor operator+(MyColor lhs, const MyColor& rhs) {
		lhs += rhs;
		return lhs;
	}

	MyColor& operator-=(const MyColor& rhs) {
		r -= rhs.r;
		g -= rhs.g;
		b -= rhs.b;
		a -= rhs.a;

		return *this;
	}

	friend MyColor operator-(MyColor lhs, const MyColor& rhs) {
		lhs -= rhs;
		return lhs;
	}
};


sf::Image generateDiff(sf::Image image) {
	sf::Image imageDiff;
	imageDiff.create(image.getSize().x, image.getSize().y);
	imageDiff.setPixel(0, 0, image.getPixel(0, 0));

	for (int y = 0; y < image.getSize().y; y++) {
		for (int x = 0; x < image.getSize().x; x++) {
		
			if (x == 0 && y == 0) {
				continue;
			}

			// Using vec4 because sf::Color will overflow since it can only handle bytes.
			Vec4 guessColorVec;
			if (x - 1 < 0) {
				MyColor col = image.getPixel(x, y - 1);
				guessColorVec = Vec4(col.r, col.g, col.b, col.a);
			}
			else if (y - 1 < 0) {
				MyColor col = image.getPixel(x - 1, y);
				guessColorVec = Vec4(col.r, col.g, col.b, col.a);
			}
			else {
				MyColor colA = image.getPixel(x - 1, y);
				MyColor colB = image.getPixel(x, y - 1);
				guessColorVec = Vec4(colA.r + colB.r, colA.g + colB.g, colA.b + colB.b, colA.a + colB.a);
				guessColorVec.a /= 2;
				guessColorVec.b /= 2;
				guessColorVec.c /= 2;
				guessColorVec.d /= 2;
			}
			MyColor guessColor(guessColorVec.a, guessColorVec.b, guessColorVec.c, guessColorVec.d);
			MyColor diffColor = MyColor(image.getPixel(x, y)) - guessColor;

			imageDiff.setPixel(x, y, diffColor);
		}
	}
	return imageDiff;
}


sf::Image generateImage(sf::Image imageDiff) {
	sf::Image image;
	image.create(imageDiff.getSize().x, imageDiff.getSize().y);
	image.setPixel(0, 0, imageDiff.getPixel(0, 0));

	for (int y = 0; y < image.getSize().y; y++) {
		for (int x = 0; x < image.getSize().x; x++) {		
			if (x == 0 && y == 0) {
				continue;
			}

			// Using vec4 because sf::Color will overflow since it can only handle bytes
			Vec4 guessColorVec;
			if (x - 1 < 0) {
				MyColor col = image.getPixel(x, y - 1);
				guessColorVec = Vec4(col.r, col.g, col.b, col.a);
			}
			else if (y - 1 < 0) {
				MyColor col = image.getPixel(x - 1, y);
				guessColorVec = Vec4(col.r, col.g, col.b, col.a);
			}
			else {
				MyColor colA = image.getPixel(x - 1, y);
				MyColor colB = image.getPixel(x, y - 1);
				guessColorVec = Vec4(colA.r + colB.r, colA.g + colB.g, colA.b + colB.b, colA.a + colB.a);
				guessColorVec.a /= 2;
				guessColorVec.b /= 2;
				guessColorVec.c /= 2;
				guessColorVec.d /= 2;
			}
			
			MyColor guessColor(guessColorVec.a, guessColorVec.b, guessColorVec.c, guessColorVec.d);
			MyColor originalColor = guessColor + MyColor(imageDiff.getPixel(x, y));

			image.setPixel(x, y, originalColor);
		}
	}
	return image;
}


int main() {

	sf::RenderWindow window(sf::VideoMode(800, 600), "Image Compression");
	window.setVerticalSyncEnabled(true);

	sf::Image image;
	if (!image.loadFromFile("dices.png")) {
		std::cout << "Failed to load file to image" << std::endl;
		return 1;
	}

	window.setSize(image.getSize());
	window.setView(sf::View(sf::FloatRect(0, 0, image.getSize().x, image.getSize().y)));

	// Generate imageDiff from original image
	sf::Image imageDiff = generateDiff(image);

	sf::Texture imageDiffTexture;
	if (!imageDiffTexture.loadFromImage(imageDiff)) {
		std::cout << "Failed to load image to texture" << std::endl;
		return 1;
	}

	sf::Sprite imageDiffSprite;
	imageDiffSprite.setTexture(imageDiffTexture);

	// Generate original image from imageDiff
	image = generateImage(imageDiff);
	sf::Texture imageTexture;
	if (!imageTexture.loadFromImage(image)) {
		std::cout << "Failed to load image to texture" << std::endl;
		return 1;
	}

	sf::Sprite imageSprite;
	imageSprite.setTexture(imageTexture);

	while (window.isOpen())
	{
		sf::Event event;

		while (window.pollEvent(event))
		{

			switch (event.type) {
				case sf::Event::Closed: {
					window.close();
				} break;

				case sf::Event::MouseButtonPressed: {
					if (event.mouseButton.button == sf::Mouse::Button::Left) {

					}
				} break;

				case sf::Event::MouseButtonReleased: {
					if (event.mouseButton.button == sf::Mouse::Button::Left) {

					}
				} break;
			}
		}

		window.clear(sf::Color::White);

		window.draw(imageDiffSprite);

		window.display();
	}



	return 0;
}