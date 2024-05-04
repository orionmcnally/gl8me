#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<array>
#include<vector>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include<windows.h>


class M3x3 {
private:
	std::array<float, 9> matrix;

public:
	M3x3() : matrix({ 1.0f, 0.0f, 0.0f,
					 0.0f, 1.0f, 0.0f,
					 0.0f, 0.0f, 1.0f }) {}

	M3x3 multiply(const M3x3& m) const {
		M3x3 output;
		output.matrix[0] = matrix[M00] * m.matrix[M00] + matrix[M10] * m.matrix[M01] + matrix[M20] * m.matrix[M02];
		output.matrix[1] = matrix[M01] * m.matrix[M00] + matrix[M11] * m.matrix[M01] + matrix[M21] * m.matrix[M02];
		output.matrix[2] = matrix[M02] * m.matrix[M00] + matrix[M12] * m.matrix[M01] + matrix[M22] * m.matrix[M02];

		output.matrix[3] = matrix[M00] * m.matrix[M10] + matrix[M10] * m.matrix[M11] + matrix[M20] * m.matrix[M12];
		output.matrix[4] = matrix[M01] * m.matrix[M10] + matrix[M11] * m.matrix[M11] + matrix[M21] * m.matrix[M12];
		output.matrix[5] = matrix[M02] * m.matrix[M10] + matrix[M12] * m.matrix[M11] + matrix[M22] * m.matrix[M12];

		output.matrix[6] = matrix[M00] * m.matrix[M20] + matrix[M10] * m.matrix[M21] + matrix[M20] * m.matrix[M22];
		output.matrix[7] = matrix[M01] * m.matrix[M20] + matrix[M11] * m.matrix[M21] + matrix[M21] * m.matrix[M22];
		output.matrix[8] = matrix[M02] * m.matrix[M20] + matrix[M12] * m.matrix[M21] + matrix[M22] * m.matrix[M22];

		return output;
	}

	M3x3 transition(float x, float y) {
		M3x3 output;
		output.matrix[0] = matrix[M00];
		output.matrix[1] = matrix[M01];
		output.matrix[2] = matrix[M02];

		output.matrix[3] = matrix[M10];
		output.matrix[4] = matrix[M11];
		output.matrix[5] = matrix[M12];

		output.matrix[6] = x * matrix[M00] + y * matrix[M10] + matrix[M20];
		output.matrix[7] = x * matrix[M01] + y * matrix[M11] + matrix[M21];
		output.matrix[8] = x * matrix[M02] + y * matrix[M12] + matrix[M22];

		return output;
	}

	M3x3 scale(float x, float y) {
		M3x3 output;
		output.matrix[0] = matrix[M00] * x;
		output.matrix[1] = matrix[M01] * x;
		output.matrix[2] = matrix[M02] * x;

		output.matrix[3] = matrix[M10] * y;
		output.matrix[4] = matrix[M11] * y;
		output.matrix[5] = matrix[M12] * y;

		output.matrix[6] = matrix[M20];
		output.matrix[7] = matrix[M21];
		output.matrix[8] = matrix[M22];

		return output;
	}
	const float* getFloatArray() {
		return &matrix[0];
	}

	static constexpr int M00 = 0;
	static constexpr int M01 = 1;
	static constexpr int M02 = 2;
	static constexpr int M10 = 3;
	static constexpr int M11 = 4;
	static constexpr int M12 = 5;
	static constexpr int M20 = 6;
	static constexpr int M21 = 7;
	static constexpr int M22 = 8;
};


class Point {
	public: 
		float x;
		float y;
		Point() { x = 0.0f; y = 0.0f; }
		Point(float h, float v) : x(h), y(v) {}
};

class Material {
	public:
		unsigned int vs;
		unsigned int fs;
		std::string VVS;
		std::string FFS;
		unsigned int program;
		unsigned int maxi;
		struct Params {
			GLenum dType = 0;
			int location = 0;
			bool uniform = 0;
			std::string name = "placeholder";
		};
		Params parameters[20];
		Material() { 
			vs = 0; fs = 0; program = 0; maxi = 0;
		};
		Material(std::string vsh, std::string fsh) {
			vs = 0; fs = 0; program = 0; maxi = 0;
			program = glCreateProgram();
			VVS = parseShader(vsh);
			FFS = parseShader(fsh);
			vs = getShader(VVS, GL_VERTEX_SHADER);
			fs = getShader(FFS, GL_FRAGMENT_SHADER);
			glAttachShader(program, vs);
			glAttachShader(program, fs);
			glLinkProgram(program);
			glValidateProgram(program);
			gatherParams();
			glDeleteShader(vs);
			glDeleteShader(fs);
		}
		unsigned int getShader(const std::string& source, unsigned int type) {
			unsigned int id = glCreateShader(type);
			const char* src = source.c_str();
			glShaderSource(id, 1, &src, nullptr);
			glCompileShader(id);
			return id;
		}
		static std::string parseShader(const std::string& filepath)  {
			std::ifstream stream(filepath);
			std::string line;
			std::stringstream ss;

			while (getline(stream, line)) {
				ss << line << '\n';
			}
	
			return ss.str();
		}
		void gatherParams() {
			unsigned int isUni = 0;
			maxi = 0;
			while (isUni < 2) {
				unsigned int paramType = isUni ? GL_ACTIVE_UNIFORMS : GL_ACTIVE_ATTRIBUTES;
				int ndx;
				glGetProgramiv(program, paramType, &ndx);

				for (int i = 0; i < ndx; i++) {
					GLenum detailsType = 0;
					int loc = 0;
					GLsizei length = 0;
					int maxL = 0;
					GLchar* name = nullptr;
					std::string tname = "placeholder";
					int size = 0;
					if (isUni) {
						glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxL);
						tname = new GLchar[maxL];
						name = &tname[0];
						glGetActiveUniform(program, i, maxL, &length, &size, &detailsType, name);
						loc = glGetUniformLocation(program, name);
					}
					else {
						glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxL);
						tname = new GLchar[maxL];
						name = &tname[0];
						glGetActiveAttrib(program, i, maxL, &length, &size, &detailsType, name);
						loc = glGetAttribLocation(program, name);
					}
					parameters[maxi].location = loc;
					parameters[maxi].uniform = isUni;
					parameters[maxi].dType = detailsType;
					parameters[maxi].name = name;
					maxi = maxi + 1;
				}
				isUni += 1;
			}
		}
		void set(std::string name, const float* m, float a, float b, float c, float d) {
			for (unsigned int p = 0; p < maxi; p++) {
				if (name == parameters[p].name) {
					if (parameters[p].uniform == true) {
						switch (parameters[p].dType) {
						case GL_FLOAT:
							glUniform1f(parameters[p].location, a);
							break;
						case GL_FLOAT_VEC2:
							glUniform2f(parameters[p].location, a, b);
							break;
						case GL_FLOAT_VEC3:
							glUniform3f(parameters[p].location, a, b, c);
							break;
						case GL_FLOAT_VEC4:
							glUniform4f(parameters[p].location, a, b, c, d);
							break;
						case GL_FLOAT_MAT3:
							glUniformMatrix3fv(parameters[p].location, 1, GL_FALSE, m);
							break;
						case GL_FLOAT_MAT4:
							glUniformMatrix4fv(parameters[p].location, 1, GL_FALSE, m);
							break;
						case GL_SAMPLER_2D:
							glUniform1i(parameters[p].location, int(a));
							break;
						}
					}
					else {
						glEnableVertexAttribArray(parameters[p].location);
						GLenum e = GL_FLOAT;
						if (a == NULL) {
							a = GL_FLOAT;
						}
						if (b == NULL) {
							b = GL_FALSE;
						}
						if (c == NULL) {
							c = 0;
						}
						if (d == NULL) {
							d = 0;
						}
						switch (parameters[p].dType) {
						case GL_FLOAT:
							glVertexAttribPointer(parameters[p].location, 1, e, b, c, (void*)0);
							break;
						case GL_FLOAT_VEC2:
							glVertexAttribPointer(parameters[p].location, 2, e, b, c, (void*)0);
							break;
						case GL_FLOAT_VEC3:
							glVertexAttribPointer(parameters[p].location, 3, e, b, c, (void*)0);
							break;
						case GL_FLOAT_VEC4:
							glVertexAttribPointer(parameters[p].location, 4, e, b, c, (void*)0);
							break;
						}
					}
				}
			}
		}
};

class Sprite {
	public:
		Material material;
		Point size;
		M3x3 oMat;
		unsigned char* image;
		GLuint glTex, texBuff, geoBuff;
		GLfloat uv_x, uv_y;
		Sprite() { image = nullptr; glTex = 0; texBuff = 0; geoBuff = 0; uv_x = 0.0f; uv_y = 0.0f; };
		Sprite(std::string url, std::string vs, std::string fs, float width, float height) : material(vs, fs), size(width, height){
			int wid = int(size.x);
			int hei = int(size.y);
			std::cout << url.c_str() << std::endl;
			int nChannels;
			stbi_set_flip_vertically_on_load(1);
			image = stbi_load(url.c_str(), &wid, &hei, &nChannels, 0);

			if (image) {
				std::cout << "loaded" << std::endl;
			}
			else {
				std::cerr << "Failed to load texture: " << url << '\n';
			}
			setup();
		}
		static GLfloat* createRectArr(float x = 0, float y = 0, float w = 1, float h = 1) {
			GLfloat* arr = new GLfloat[12];
			arr[0] = x; arr[4] = x; arr[6] = x;
			arr[1] = y; arr[3] = y; arr[9] = y;
			arr[2] = x+w; arr[8] = x+w; arr[10] = x+w;
			arr[5] = y+h; arr[7] = y+h; arr[11] = y+h;
			return arr;
		}
		void setup() {
			glUseProgram(material.program);
			glGenTextures(1, &glTex);
			glBindTexture(GL_TEXTURE_2D, glTex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
			glBindTexture(GL_TEXTURE_2D, 0);

			stbi_image_free(image);

			uv_x = 1;
			uv_y = 1;

			glGenBuffers(1, &texBuff);
			glBindBuffer(GL_ARRAY_BUFFER, texBuff);
			glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), createRectArr(0, 0, uv_x, uv_y), GL_STATIC_DRAW);

			glGenBuffers(1, &geoBuff);
			glBindBuffer(GL_ARRAY_BUFFER, geoBuff);
			glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), createRectArr(0, 0, size.x, size.y), GL_STATIC_DRAW);

			glUseProgram(NULL);
		}
		void render(const float* wsMat, float posX, float posY, unsigned int frameX, unsigned int frameY, float scalex, float scaley, std::array<float, 4> col, bool ignore) {
			if (!ignore) {
				if (posX == NULL)
					posX = 0.0f;
				if (posY == NULL)
					posY = 0.0f;
				if (scalex == NULL)
					scalex = 1.0f;
				if (scaley == NULL)
					scaley = 1.0f;
				if (frameX == NULL)
					frameX = 0;
				if (frameY == NULL)
					frameY = 0;
				

				oMat.transition(posX, posY);

				glUseProgram(material.program);
				material.set("u_color", NULL, col[0], col[1], col[2], col[3]);

				oMat.scale(scalex, scaley);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, glTex);
				material.set("u_img", NULL, 0.0f, NULL, NULL, NULL);

				glBindBuffer(GL_ARRAY_BUFFER, texBuff);
				material.set("a_texCoord", NULL, NULL, NULL, NULL, NULL);
				
				glBindBuffer(GL_ARRAY_BUFFER, geoBuff);
				material.set("a_position", NULL, NULL, NULL, NULL, NULL);

				material.set("u_frame", NULL, frameX, frameY, NULL, NULL);
				material.set("u_world", wsMat, NULL, NULL, NULL, NULL);
				material.set("u_object", oMat.getFloatArray(), NULL, NULL, NULL, NULL);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

				glUseProgram(0);
			}
		}
};

class Renderablee {
	public:
		Sprite* sprite = (nullptr);
		float posX;
		float posY;
		int frameX;
		int frameY;
		float scalex;
		float scaley;
		std::array<float, 4> col;
		bool ignore;
		Renderablee() {
			posX = 0.0f;
			posY = 0.0f;
			frameX = 0;
			frameY = 0;
			scalex = 1.0f;
			scaley = 1.0f;
			col[0] = 1.0f; col[1] = 1.0f; col[2] = 1.0f; col[3] = 1.0f;
			ignore = true;
		};
		Renderablee(Sprite* spr, float x, float y, int fx, int fy, float sx, float sy, float r, float g, float b, float a, bool ignor) {
			sprite = spr;
			posX = x; posY = y;
			frameX = fx; frameY = fy;
			scalex = sx; scaley = sy;
			col[0] = r; col[1] = g; col[2] = b; col[3] = a;
			ignore = ignor;
		}
		Renderablee(Sprite* spr, float x, float y, int fx, int fy, float sx, float sy, bool ignor) {
			sprite = spr;
			posX = x; posY = y;
			frameX = fx; frameY = fy;
			scalex = sx; scaley = sy;
			col[0] = 1.0f; col[1] = 1.0f; col[2] = 1.0f; col[3] = 1.0f;
			ignore = ignor;
		}
		Renderablee(Sprite* spr, float x, float y, int fx, int fy, float sx, float sy, float r, float g, float b, float a) {
			sprite = spr;
			posX = x; posY = y;
			frameX = fx; frameY = fy;
			scalex = sx; scaley = sy;
			col[0] = r; col[1] = g; col[2] = b; col[3] = a;
			ignore = false;
		}
		Renderablee(Sprite* spr, float x, float y, int fx, int fy, float sx, float sy) {
			sprite = spr;
			posX = x; posY = y;
			frameX = fx; frameY = fy;
			scalex = sx; scaley = sy;
			col[0] = 1.0f; col[1] = 1.0f; col[2] = 1.0f; col[3] = 1.0f;
			ignore = false;
		}
		Renderablee(Sprite* spr, float x, float y, int fx, int fy) {
			sprite = spr;
			posX = x; posY = y;
			frameX = fx; frameY = fy;
			scalex = 1.0f; scaley = 1.0f;
			col[0] = 1.0f; col[1] = 1.0f; col[2] = 1.0f; col[3] = 1.0f;
			ignore = false;

		};
		
};

class Layer {
	public: 
		Renderablee* renderables;
		int blendmode;
		Layer(Renderablee* rptr) {
			renderables = rptr;
			blendmode = 0;
		}
};

class BackBuffer {
	public:
		Point size;
		GLuint fBuff{}, rBuff{}, tex{}, tBuff{}, gBuff{};
		Material material;
		BackBuffer(std::string vsh, std::string fsh, float width, float height) : material(vsh, fsh), size(width, height) {
			glGenFramebuffers(1, &fBuff);
			glGenRenderbuffers(1, &rBuff);
			glGenTextures(1, &tex);

			glBindFramebuffer(GL_FRAMEBUFFER, fBuff);
			glBindRenderbuffer(GL_RENDERBUFFER, rBuff);
			glBindTexture(GL_TEXTURE_2D, tex);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size.x, size.y);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rBuff);

			glGenBuffers(1, &tBuff);
			glBindBuffer(GL_ARRAY_BUFFER, tBuff);
			glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), Sprite::createRectArr(0, 0, 1, 1), GL_STATIC_DRAW);

			glGenBuffers(1, &gBuff);
			glBindBuffer(GL_ARRAY_BUFFER, gBuff);
			glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), Sprite::createRectArr(-1, -1, 2, 2), GL_STATIC_DRAW);

			glBindTexture(GL_TEXTURE_2D, 0);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glBindTexture(GL_TEXTURE_2D, 0);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void render() {
			glUseProgram(material.program);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex);
			material.set("u_img", NULL, 0, NULL, NULL, NULL);

			glBindBuffer(GL_ARRAY_BUFFER, tBuff);
			material.set("a_texCoord", NULL, NULL, NULL, NULL, NULL);

			glBindBuffer(GL_ARRAY_BUFFER, gBuff);
			material.set("a_position", NULL, NULL, NULL, NULL, NULL);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
			glUseProgram(0);
		}
};

class GameL {
	public:
		float wRatio = 0.0f, arby = 960.0f;
		float rArrL = 0.0f, lArrL = 0.0f;
		bool ready = false;
		M3x3 wsMat;
		const float* wsptr;
		Point size;
		GLFWwindow* window;
		BackBuffer bb, fb;
		Sprite sprites;
		Renderablee renderables;
		Layer layers;
		GameL(GLFWwindow* windo) : bb("res/BBVS.glvs", "res/BBFS.glfs", 4096, 4096), fb("res/BBVS.glvs", "res/BBFS.glfs", 4096, 4096), sprites("ass/0.png", "res/VS1.glvs", "res/FS1.glfs", 32, 32), renderables(&sprites, 200.0f, 200.0f, 0, 0, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, false), layers(&renderables) {
			window = windo;
			wsptr = wsMat.getFloatArray();
			glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			glEnable(GL_BLEND);
			//sprites = Sprite("ass/0.png", "res/VS1.glvs", "res/FS1.glfs", 32, 32);
			//renderables = Renderablee(&sprites, 1.0f, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, false);
			//layers = Layer(&renderables);
			//gatherSprites();
			
		}
		//void gatherSprites() {
			
		//	gatherRenderables();
		//}
		void gatherRenderables() {
			
			//layers[0].renderables[0] = &renderables[0];
			ready = true;
			//std::cout << layers[0].renderables[0].ignore << std::endl;

		}
		const float* getFloatArray() const {
			return wsptr;
		}
		void setBuffer(BackBuffer buff, bool tog) {
			if (tog) {
				glViewport(0, 0, buff.size.x, buff.size.y);
				glBindFramebuffer(GL_FRAMEBUFFER, buff.fBuff);
			}
			else {
				glViewport(0, 0, size.x, size.y);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		}
		void setBlendmode(int bm) {
			switch (bm) {
			case 0:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case 1: 
				glBlendFunc(GL_ONE, GL_ONE);
				break;
			case 2:
				glBlendFunc(GL_DST_COLOR, GL_ZERO);
				break;
			}
		}
		void resize(int x, int y) {
			size.x = x; size.y = y;
			glfwGetFramebufferSize(window, &x, &y);
			glViewport(0, 0, x, y);
			wRatio = x / (y / arby);
			wsMat.transition(-1.0f, 1.0f);
			wsMat.scale(2.0f / wRatio, -2.0f / arby);
		}
		void update() {


			//render
			rArrL = 1; // sizeof(layers[0].renderables) / sizeof(layers[0].renderables[0]);
			lArrL = 1; // sizeof(layers) / sizeof(layers[0]);
			glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

			/*for (int i = 0; i < lArrL; i++) {
				setBuffer(bb, 1);
				if (i == 0)
					glClear(GL_COLOR_BUFFER_BIT);
				for (int j = 0; j < rArrL; j++) {
					setBlendmode(0);
					if (layers[i].renderables[j].ignore == false)
						layers[i].renderables[j].sprite->render(wsptr, layers[i].renderables[j].posX, layers[i].renderables[j].posY, layers[i].renderables[j].frameX, layers[i].renderables[j].frameY, layers[i].renderables[j].scalex, layers[i].renderables[j].scaley, layers[i].renderables[j].col, layers[i].renderables[j].ignore);
				}
				setBlendmode(layers[i].blendmode);
				setBuffer(fb, 1);
				bb.render();
			}*/
			for (int i = 0; i < lArrL; i++) {
				setBuffer(bb, 1);
				if (i == 0)
					glClear(GL_COLOR_BUFFER_BIT);
				for (int j = 0; j < rArrL; j++) {
					setBlendmode(0);
					if (layers.renderables->ignore == false)
						layers.renderables->sprite->render(wsptr, layers.renderables->posX, layers.renderables->posY, layers.renderables->frameX, layers.renderables->frameY, layers.renderables->scalex, layers.renderables->scaley, layers.renderables->col, layers.renderables->ignore);
				}
				setBlendmode(layers.blendmode);
				setBuffer(fb, 1);
				bb.render();
			}
			setBuffer(bb, 0);
			//glClear(GL_COLOR_BUFFER_BIT);
			setBlendmode(0);
			fb.render();
			glFlush();
		}
};

int main()
{
			int sw = GetSystemMetrics(SM_CXSCREEN);
			int sh = GetSystemMetrics(SM_CYSCREEN);

			glfwInit();
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			GLFWwindow* window = glfwCreateWindow(sw, sh, "Dicknballs", NULL, NULL);
			if (window == NULL)
			{
				std::cout << "Rekt" << std::endl;
				glfwTerminate();
				return -1;
			}
			glfwMakeContextCurrent(window);
			gladLoadGL();
			glViewport(0, 0, sw, sh);
			GameL Game(window);
			Game.resize(sw, sh);
			while (!glfwWindowShouldClose(window)) {
				
				Game.update();
				glfwSwapBuffers(window);
				glfwPollEvents();
			}
}



//static std::string parseShader(const std::string& filepath)
//{
//	std::ifstream stream(filepath);
//	std::string line;
//	std::stringstream ss;
//
//	while (getline(stream, line))
//	{
//		ss << line << '\n';
//	}
//	
//	return ss.str();
//}
//
//static unsigned int compileShader(const std::string& source, unsigned int type)
//{
//	unsigned int id = glCreateShader(type);
//	const char* src = source.c_str();
//	glShaderSource(id, 1, &src, nullptr);
//	glCompileShader(id);
//
//	return id;
//}
//
//static unsigned int createShader(const std::string& vsh, const std::string& fsh)
//{
//	unsigned int program = glCreateProgram();
//	unsigned int vs = compileShader(vsh, GL_VERTEX_SHADER);
//	unsigned int fs = compileShader(fsh, GL_FRAGMENT_SHADER);
//	glAttachShader(program, vs);
//	glAttachShader(program, fs);
//	glLinkProgram(program);
//	glValidateProgram(program);
//
//	glDeleteShader(vs);
//	glDeleteShader(fs);
//
//	return program;
//}
//
//int main()
//{
//	glfwInit();
//
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//	float verts[] =
//	{
//		-0.5f, -0.5f, 0.0f,
//		0.5f, -0.5f, 0.0f,
//		0.0f, 0.5f, 0.0f
//	};
//
//	GLFWwindow* window = glfwCreateWindow(800, 800, "Dicknballs", NULL, NULL);
//	if (window == NULL)
//	{
//		std::cout << "Rekt" << std::endl;
//		glfwTerminate();
//		return -1;
//	}
//	glfwMakeContextCurrent(window);
//
//	gladLoadGL();
//	glViewport(0, 0, 800, 800);
//
//	std::string VSS0 = parseShader("res/VS0.glvs");
//	std::string FSS0 = parseShader("res/FS0.glfs");
//	std::cout << VSS0 << std::endl;
//	std::cout << FSS0 << std::endl;
//
//	unsigned int shader = createShader(VSS0, FSS0);
//	std::cout << shader << std::endl;
//	unsigned int VAO, VBO;
//	glGenVertexArrays(1, &VAO);
//	glGenBuffers(1, &VBO);
//	glBindVertexArray(VAO);
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindVertexArray(0);
//
//	
//
//	
//	
//
//	while (!glfwWindowShouldClose(window))
//	{
//		
//		glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT);
//		glUseProgram(shader);
//		glBindVertexArray(VAO);
//		glDrawArrays(GL_TRIANGLES, 0, 3);
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//	}
//
//	glfwDestroyWindow(window);
//	glfwTerminate();
//	return 0;
//}