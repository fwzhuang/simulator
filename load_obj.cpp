
#include "load_obj.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <FreeImage.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

using namespace std;



Obj::Obj(const string file, cloth_type type):obj_file(file),obj_type(type)
{
	ifstream input(file);
	if(!input)
	{
		cout <<"error: unable to open input file: " << endl;
		exit(-1);
	}

	while (!input.eof())
	{
		string buffer;
		getline(input, buffer);
		stringstream line(buffer);
		string	f0 ,f1, f2, f3;
		line >> f0 >> f1 >> f2 >> f3;

		if(f0 == "mtllib")
			mtl_file = f1;
		else if (f1 == "object")
		{
			vertex_object.push_back(make_pair(f2,0));
		}
		else if (f2 == "vertices")
		{
			vertex_object.back().second = atoi(f1.c_str());
		}
		else if(f0 == "v")
		{
			glm::vec4 ver(atof(f1.c_str()),atof(f2.c_str()),atof(f3.c_str()),1.0); 
			vertices.push_back(ver);

		}
		else if(f0 == "vn")
		{
			glm::vec3 nor(atof(f1.c_str()),atof(f2.c_str()),atof(f3.c_str())); 
			normals.push_back(nor);
		}
		else if(f0 == "vt")
		{
			glm::vec2 tex_coords(atof(f1.c_str()),atof(f2.c_str()));
			tex.push_back(tex_coords);
		}
		else if(f0 == "g")   //read each group
		{
			face_group.push_back(make_pair(f1,0));
		}
		else if(f2 == "polygons")
		{
			stringstream tem_line(buffer);
			string	f00 ,f11, f22, f33,f44;
			tem_line >> f00 >> f11 >> f22 >> f33 >> f44;
			face_group.back().second = atoi(f44.c_str());
		}
		else if(f0 == "f")
		{
			Face tem;
			int sPos = 0;
			int ePos = sPos;
			string temp;
			ePos = f1.find_first_of("/");
			if(ePos == string::npos)  //处理不同格式的face, f  1 2 3
			{
				tem.vertex_index[0] = atoi(f1.c_str()) - 1;
				tem.vertex_index[1] = atoi(f2.c_str()) - 1;
				tem.vertex_index[2] = atoi(f3.c_str()) - 1;

				tem.tex_index[0] = 0;     //add default tex
				tem.tex_index[1] = 1;
				tem.tex_index[2] = 2;

				tem.normal_index[0] = atoi(f1.c_str()) - 1;
				tem.normal_index[1] = atoi(f2.c_str()) - 1;
				tem.normal_index[2] = atoi(f3.c_str()) - 1;

			}
			else     //处理不同格式的face, f  1/1/1 2/2/2 3/3/3
			{
				if(ePos != string::npos)  {
					temp = f1.substr(sPos, ePos - sPos);
					tem.vertex_index[0] = atoi(temp.c_str()) - 1;

					sPos = ePos+1;
					ePos = f1.find("/", sPos);
					temp = f1.substr(sPos, ePos - sPos);
					tem.tex_index[0] = atoi(temp.c_str()) - 1;

					sPos = ePos+1;
					ePos = f1.length();
					temp = f1.substr(sPos, ePos - sPos);
					tem.normal_index[0] = atoi(temp.c_str()) - 1;
				}

				sPos = 0;
				ePos = f2.find_first_of("/");
				if(ePos != string::npos)  {
					temp = f2.substr(sPos, ePos - sPos);
					tem.vertex_index[1] = atoi(temp.c_str()) - 1;

					sPos = ePos + 1;
					ePos = f2.find("/", sPos+1);
					temp = f2.substr(sPos, ePos - sPos);
					tem.tex_index[1] = atoi(temp.c_str()) - 1;

					sPos = ePos + 1;
					ePos = f2.length();
					temp = f2.substr(sPos, ePos - sPos);
					tem.normal_index[1] = atoi(temp.c_str()) - 1;
				}

				sPos = 0;
				ePos = f3.find_first_of("/");
				if(ePos != string::npos)  {
					temp = f3.substr(sPos, ePos - sPos);
					tem.vertex_index[2] = atoi(temp.c_str()) - 1;

					sPos = ePos + 1;
					ePos = f3.find("/", sPos+1);
					temp = f3.substr(sPos, ePos - sPos);
					tem.tex_index[2] = atoi(temp.c_str()) - 1;

					sPos = ePos + 1;
					ePos = f3.length();
					temp = f3.substr(sPos, ePos - sPos);
					tem.normal_index[2] = atoi(temp.c_str()) - 1;
				}
			}

			faces.push_back(tem);
		}
	}

	if(!mtl_file.empty())
	{
		// read material, here we just read the texture and only 1 photo

		string path = obj_file.substr(0,obj_file.find_last_of("/")+1);
		mtl_file = path + mtl_file;
		ifstream input(mtl_file);
		if(!input)
		{
			cerr<<"error: unable to open input file: "<<endl;
			exit(-1);
		}
		
		while (!input.eof())
		{
			string buffer;
			getline(input, buffer);
			stringstream line(buffer);
			string	f0 ,f1;
			line >> f0 >> f1;
			if(f0 == "map_Ka")
			{
				texture_file = f1;
				break;
			}	
		}
	}


	if(!texture_file.empty()) // load .png and you need to initilize opengl before load
	{
		string path = obj_file.substr(0,obj_file.find_last_of("/")+1);
		texture_file = path + texture_file;
		FIBITMAP *dib = FreeImage_Load(FIF_PNG,texture_file.c_str(), PNG_DEFAULT);

		if (FreeImage_GetBPP(dib) != 32) 
		{
			FIBITMAP* tempImage = dib;
			dib = FreeImage_ConvertTo32Bits(tempImage);
		}

		if (dib != NULL)
		{
			glGenTextures(1, &g_textureID);
			glBindTexture(GL_TEXTURE_2D, g_textureID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			BYTE *bits = new BYTE[FreeImage_GetWidth(dib) * FreeImage_GetHeight(dib) * 4];
			BYTE *pixels = (BYTE*) FreeImage_GetBits(dib);

			for (int pix = 0; pix<FreeImage_GetWidth(dib) * FreeImage_GetHeight(dib); pix++)
			{
				bits[pix * 4 + 0] = pixels[pix * 4 + 2];
				bits[pix * 4 + 1] = pixels[pix * 4 + 1];
				bits[pix * 4 + 2] = pixels[pix * 4 + 0];
				bits[pix * 4 + 3] = pixels[pix * 4 + 3]; 
			}

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FreeImage_GetWidth(dib), FreeImage_GetHeight(dib), 0,GL_RGBA, GL_UNSIGNED_BYTE, bits);

			FreeImage_Unload(dib);
			delete bits;
		}
		cout << file << " load successfully!" << endl;
	}

	cout << "vertices size" << vertices.size() << endl;

}

Obj::~Obj()
{}


void Obj::scale_translate(float S, float x_up, float y_up, float z_up)
{
	//获取模型中心坐标
	glm::vec3 center = get_center();
	//const float up = 1.2;
	int n = vertices.size();
	for (int i = 0; i<n; ++i)
	{
		vertices[i].x = (vertices[i].x - center.x) / S; vertices[i].x += x_up;
		vertices[i].y = (vertices[i].y - center.y) / S; vertices[i].y += y_up;
		vertices[i].z = (vertices[i].z - center.z) / S; vertices[i].z += z_up;
	}
}

void Obj::unified()
{
	uni_vertices = vertices;
	uni_tex.resize(uni_vertices.size());
	uni_normals.resize(uni_vertices.size());
	vertex_index.resize(faces.size() * 3);

	for(int i=0;i<faces.size();i++)
	{
		for(int j=0;j<3;j++)
		{
			uni_tex[faces[i].vertex_index[j]] = tex[faces[i].tex_index[j]];
			uni_normals[faces[i].vertex_index[j]] = normals[faces[i].normal_index[j]];  
			vertex_index[i*3+j] = faces[i].vertex_index[j]; 
		}
	}
}

cloth_type Obj::get_obj_type()
{
	return obj_type;
}

void Obj::rotation(float angle, direction dir)
{
	angle = angle / 180 * 3.1415;
	glm::vec3 center = get_center();
	glm::mat4x4 R_matrix;
	if (dir == X)
		R_matrix = { {1.0, 0.0, 0.0, 0.0},
					{0.0, cos(angle), -sin(angle), 0.0},
					{0.0, sin(angle), cos(angle), 0.0},
					{0.0, 0.0, 0.0, 1.0}
				   };
	else if(dir == Y)
	{
		R_matrix = { {cos(angle), 0, -sin(angle), 0},
					{0, 1.0, 0, 0},
					{sin(angle), 0, cos(angle), 0},
					{0, 0, 0, 1.0}
					};
	}
	else
	{
		R_matrix = { {cos(angle), -sin(angle), 0, 0},
					{sin(angle), cos(angle), 0, 0},
					{0, 0, 1, 0},
					{0, 0, 0, 1}
					};
	}

	int n = vertices.size();
	for (auto& vertex : vertices)
	{
		vertex -= glm::vec4(center, 0.0);
		vertex = R_matrix*vertex;
		vertex += glm::vec4(center, 0.0);
	}

}

glm::vec3 Obj::get_center()
{
	glm::vec3 center;
	int n = vertices.size();
	float sumx = 0, sumy = 0, sumz = 0;
	for (int i = 0; i<n; ++i)
	{
		sumx += vertices[i].x;
		sumy += vertices[i].y;
		sumz += vertices[i].z;
	}
	center.x = sumx / n;
	center.y = sumy / n;
	center.z = sumz / n;
	return center;
}

void Obj::vertex_extend(float dist)
{
	vector<bool> visited(vertices.size(),false);
	for (int i = 0; i < faces.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			int v_index = faces[i].vertex_index[j];
			if (visited[v_index] == false)
			{
				glm::vec3 n = normals[faces[i].normal_index[j]];
				vertices[v_index] += dist*glm::vec4(n,0);
				visited[v_index] = true;
			}
		}
	}
}

void Obj::save()
{
	ofstream outfile("../tem/body.obj");

	outfile << "# vertices" << endl;
	for (auto ver : uni_vertices)
	{
		outfile << "v " << ver.x << " " << ver.y << " " << ver.z << endl;   //数据写入文件
	}

	outfile << "# faces" << endl;
	for (auto face : faces)
	{
		outfile << "f " << face.vertex_index[0] + 1 << " " << face.vertex_index[1] + 1 << " " << face.vertex_index[2] + 1 << endl;
	}

	outfile.close();
}