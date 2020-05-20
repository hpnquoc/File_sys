#include "Volume.h"
//
//
//string Volume::getName16Char(const string& source)
//{
//	string name = source;
//	if (name.length() >= 16)
//	{
//		name = name.substr(0, 16);
//	}
//	else
//	{
//		for (int i = name.length(); i < 16; i++)
//		{
//			name += " ";
//		}
//	}
//
//	return name;
//}
//
//Volume::Volume(const string& name, const int& size, const int& startCluster, const int& number)
//{
//	this->name = this->getName16Char(name);
//	this->size = size;
//	this->remaining = size;
//	this->startCluster = startCluster;
//	this->number = number;
//}
//
//void Volume::InitVolume(const string& disk)
//{
//	fstream fout(disk, ios_base::binary | ios_base::out | ios_base::in);
//	if (fout.is_open() == false)
//	{
//		throw exception("Can't open disk. Can't init volume");
//	}
//
//	//Go to position at cluster 1
//	fout.seekp(8 * 512 + 4 * number, ios_base::beg);
//	fout.write((char*)&startCluster, 4);
//
//	//Go to this start cluster
//	fout.seekp(8 * 512 * startCluster, ios_base::beg);
//	fout << name;
//	fout.write((char*)&startCluster, 4);
//	fout.write((char*)&size, 4);
//	fout.write((char*)&remaining, 4);
//
//	fout.close();
//}
#include <iostream>
#include <fstream>
#include "img.h"
#include "Debug.h"
#include "mask.h"
using namespace std;

uint32_t ConvertTimeUnixToFAT(time_t a)
{
	tm *b = new tm;
	localtime_s(b, &a);
	char day = 0;
	day = day ^ b->tm_mday;
	day = day ^ ((b->tm_mon + 1) << 5);
	day = day ^ ((b->tm_year - 80) << 9);
	char time = 0;
	time = time ^ (b->tm_sec / 2);
	time = time ^ ((b->tm_min) << 5);
	time = time ^ ((b->tm_hour) << 11);
	delete b;
	return (uint32_t)day ^ (time << 16);
}
bool Volume::Create(Packg& scope,string fileName)
{
	uint64_t max_size = scope.end - scope.strt + 1;
	max_size *= 512;
	uint64_t byte;
	do
	{
		cout << "nhap kich thuoc: (MB)";
		cin >> byte;
		byte = byte * 1024 * 1024;
	} while (byte > max_size && cout << "Bruh!\n");
	cout << "Nhap volume name";
	this->Name = getchar();
	this->setFlags();
	this->Ss = UNIT_SIZE;
	this->Sc = 8;
	this->Sb = 1;
	this->Nf = 1;
	this->Sv = byte * 2;
	this->Nc = floorf((float)(Sv - Sb) / (4 * Nf / Ss + Sc)); // thuat toan chua toi uu lam
	this->Sf = ceil((Sc * Nc) / 512) / Nf;
	this->FAT_len = Nc;
	this->startSector = scope.strt;

	ofstream fout(fileName, ios::in | ios::out | ios::binary);
	if (!fout.is_open()) {
		return EXIT_FAILURE;
	}
	seeker pos = startSector; pos = pos * 512;
	fout.seekp(pos);
	// luu volume entry == > root luu entry neu create success
	//SaveByte(fout,this->Name);
	//SaveByte(fout, start);
	//SaveByte(fout, end);
	// luu volume
	// boot
	SaveByte(fout, this->Ss);
	SaveByte(fout, this->Sc);
	SaveByte(fout, this->Sb);
	SaveByte(fout, this->Nf);
	SaveByte(fout, this->Sf);
	SaveByte(fout, this->Sv);
	SaveByte(fout, this->Nc);
	SaveByte(fout, this->StCluster);
	SaveByte(fout, this->FAT_len);

	int core = sizeof(Ss) +
		sizeof(Sc) +
		sizeof(Sb) +
		sizeof(Nf) +
		sizeof(Sf) +
		sizeof(Sv) +
		sizeof(Nc) +
		sizeof(StCluster) +
		sizeof(FAT_len);

	fout.close();
	return EXIT_SUCCESS;
}

void Volume::setFlags()
{
}

uint32_t Volume::FreeInFAT() // tra ve don vi cluster
{
	int i;
	for (i = 0; i < FAT_len; i++)
	{
		if (FAT[i] == 0)
			return i;
	}
	throw ("Volume is full");
	return 0;
}
uint32_t Volume::FreeInFAT(int i) // tra ve don vi cluster
{
	for (; i < FAT_len; i++)
	{
		if (FAT[i] == 0)
			return i;
	}
	throw ("Volume [i,max] is full");
	return 0;
}

void Volume::addEntrySt(Entry &file,Entry *ViTriRDET)
{
	if (ViTriRDET == NULL)
	{
		file.entryStCluster = (startSector + Sb + Sf * Nf + 8) * 512; // vi tri RDET chinh
		AddEntry(file);
	}
	else
	{
		ViTriRDET->list.push_back(file);
		file.entryStCluster = ViTriRDET->StCluster;
		AddEntry(file);
	}
}


seeker Volume::AddTable(seeker seek, bool End)
{
	fstream fout(disk, ios_base::in | ios_base::out | ios_base::binary);
	if (fout.is_open() == false)
	{
		throw exception("Can't open disk. Can't add table!");
	}
	fout.seekp(seek, ios::beg);
	int i = FreeInFAT();
	seeker sker = (startSector + Sb + Sf * Nf + 8) * 512 + i * Sc * Ss;
	if (End) // neu la flag end
	{
		fout.seekp(sker, ios::beg);
		SaveByte(fout, SUB_ENTRY);
		SaveByte(fout, (uint8_t)0);
		fout.seekp(16, ios::cur);
		SaveByte(fout, (uint64_t)sker);
	}
	fout.close();
	return sker;
}

void Volume::ExportFiLe(string path,const Entry * file)
{
	// tao ra file do
	// toi cho data cua file do
	// luu ra
	// dong file
}

void Volume::AddEntry(const Entry& entry)
{
	fstream fout(disk, ios_base:: in | ios_base::out | ios_base:: binary);
	if(fout.is_open() == false)
	{
		throw exception("Can't open disk. Can't add entry!");
	}

	seeker sker = entry.entryStCluster * this->Sc * this->Ss;
	fout.seekp(sker, ios::beg);

	uint8_t flag;
	do
	{
		LoadByte(fout, flag);
		//Check if(flag = END)
		if(flag ^ END < flag)
		{
			sker = AddTable(sker,NULL);
			break; 
		}
		if(flag == 0 || (flag^DELETED < flag))
		{
			break;
		}
		else
		{
			sker += 32;
		}
	} while(true);
	fout.seekp(sker);
	SaveByte(fout, entry.flags);
	SaveByte(fout, entry.ctime);
	SaveByte(fout, entry.mtime);
	SaveByte(fout, entry.StCluster);
	SaveByte(fout, entry.size);
	SaveByte(fout, entry.TypeNum);
	SaveByte(fout, entry.ino);
	SaveByte(fout, entry.entryStCluster);

	fout.close();
}

uint64_t Volume::ViTriCluster(int i) // vi tri la thu tu sector trong disk
{
	return uint64_t();
}

void Volume::AddData(fstream &file, Entry *&f)
{
	ofstream log("log.txt");
	ifstream log1("log.txt");
	ofstream Disk(disk);
	if(!Disk.is_open())
		throw exception("Can't open disk. Can't add data!");
	char *temp = new char[(Sc - 1) * Ss + 1];
	int i = FreeInFAT();
	log << i;
	seeker vt1 = ViTriCluster(i);
	seeker vt2;
	Disk.seekp(vt1, ios::beg);
	do
	{
		file.getline(temp, (Sc - 1) * Ss);
		SaveByte(Disk, f->ino);
		i = FreeInFAT(i);
		SaveByte(Disk,(uint32_t) i);
		SaveByte(Disk, f->Namesize);
		SaveByte(Disk, f->name);
		Disk.seekp(vt1 + 512, ios::beg);
		Disk.write(temp, (Sc - 1) * Ss);
		vt2 = ViTriCluster(i);
		log << i;
		log << " ";
		Disk.seekp(vt2 - vt1, ios::cur);
		vt1 = vt2;
	} while (!file.eof());
	log.close();
	log1 >> i;
	f->StCluster = i;
	FAT[i] = 1;
	while (log1 >> i)
	{
		log1.get();
		FAT[i] = 1;
	}
	log1.close();
}

bool Volume::Import(string pathFile, Entry *vitri) //luc dau vitri = NULL
{
	fstream fin(pathFile, ios_base::in | ios_base::binary);
	if (fin.is_open()) // la file
	{
		Entry a;
		int i = 0; // tim name trong path
		int temp = pathFile.find('\\', i);
		while (temp > i)
		{
			i = pathFile.find('\\', i + 1);
			temp = pathFile.find('\\', i + 1);
		}
		a.size = 1; 
		a.name = pathFile.substr(i + 1, pathFile.size() - i - 1);
		a.Namesize = a.name.size();
		a.flags = 0;
		struct stat st;
		stat(pathFile.c_str(), &st);
		a.ctime = ConvertTimeUnixToFAT(st.st_ctime);
		a.mtime = ConvertTimeUnixToFAT(st.st_mtime);
		addEntrySt(a, vitri);  // o dia
		vitri->list.push_back(a); //logic
		fin.close();
	}
	else // la thu muc hoac khong ton tai
	{
		system(("dir /b/a-d-h" + pathFile + ">file.txt").c_str()); // doc cac file
		ifstream file("file.txt");
		system(("dir /b/ad-h" + pathFile + ">folder.txt").c_str()); // doc cac file
		ifstream folder("folder.txt");
		if (!file.is_open() || !folder.is_open())
			return false;

		if (file.eof() && folder.eof())
			return false; // khong ton tai hoac thu muc trong nen khong tao

		Entry a;
		int i = 0; // tim name trong path
		int temp = pathFile.find('\\', i);
		while (temp > i)
		{
			i = pathFile.find('\\', i + 1);
			temp = pathFile.find('\\', i + 1);
		}
		a.name = pathFile.substr(i + 1, pathFile.size() - i - 1);
		a.Namesize = a.name.size();
		a.size = 0;
		a.StCluster = AddTable(0, 0);
		FAT[a.StCluster] = 1;
		struct stat st;
		stat(pathFile.c_str(), &st);
		a.ctime = ConvertTimeUnixToFAT(st.st_ctime);
		a.mtime = ConvertTimeUnixToFAT(st.st_mtime);
		addEntrySt(a, vitri); // o dia
		vitri->list.push_back(a); // logic
		Entry *link = &vitri[vitri->list.size() - 1];

		string dir;
		while (!file.eof())
		{
			getline(file, dir);
			if (!Import(pathFile + "\\" + dir, link))
				return false;
		}
		file.close();

		while (!folder.eof())
		{
			getline(folder, dir);
			if (!Import(pathFile + "\\" + dir, link))
				return false;
		}
		file.close();
		folder.close();
	}
	return 1;
}

bool Volume::Export(string path, Entry *vitri)
{
	string temp;
	if (path.size() == 0)
		temp = vitri->name;
	else
		temp = path + "\\" + vitri->name;
	if (vitri->list.size() != 0)
	{
		if (_mkdir(temp.c_str()) != 0)
		{
			return false; // tao thu muc khong thanh cong
		}
		for (int i = 0; i < vitri->list.size(); i++)
		{
			Export(temp, &vitri->list[i]);
		}
	}
	else
	{
		ExportFiLe(path, vitri);
	}
	return true;
}
