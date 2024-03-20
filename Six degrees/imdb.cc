#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"

using namespace std;

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

struct info{
  const void* name;
  const void* data;
  
};

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

int compareFilms(const void* movie, const void* offset){

  info* testFilm = (info*) movie;
  const char* file = (char*) testFilm->data;
  film compareFilmOne = *(film*)testFilm->name;
  film compareFilmTwo;
  char* index = (char*) file + *(int*)offset;
  string name = "";
  while(*index != '\0'){
    name+= *index;
    index++;
   
  }
  int year = 1900 + *(index+1);
 // cout << name << "   " << year  << "   "  << compareFilmOne.title << "   " << compareFilmOne.year << endl;
  compareFilmTwo.title = name;
  
  compareFilmTwo.year  = year;
 
  if(compareFilmOne==(compareFilmTwo))
    return 0;
  if(compareFilmOne<(compareFilmTwo))
    return -1;
  else 
    return 1;

}

int compareActors(const void* actor, const void* offset){

  info* testActor = (info*) actor;
  int bitOffset = *(int*)offset;
  const char* name = (const char*) testActor->name;
  const char* file = (char*) testActor->data;
  return strcmp(name, file + bitOffset);

}
/*
helper function for getCredits
loops over the offsets of the given actor/actress
*/
void imdb::updateVector(vector<film>& films, const void* actorOffset) const{
 char* index = (char*) actorFile + *(int*) actorOffset;
    int count = 0;
    while(*(index)!= '\0'){
     
      count++;
      index++;
    }
    index++;
    count++;
  
    if(count%2 == 1){
      index++;
      count++;
    }
    short numOfMovies = *(short*)index;
 
    count += sizeof(short);
    index += sizeof(short) + count % 4;
  
    for(int i = 0; i < numOfMovies; i++){
      int movieOffset = *((int*) index + i);
      char* movie = (char*)movieFile + movieOffset;
      string movieName = "";
      while (*(char*) movie != '\0') {
        movieName += *(char*) movie;
        movie++;
      }
      movie++;
      film f; f.title = movieName;
      f.year = 1900 + (int) (*(char*) movie);
      films.push_back(f);
    }


}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const { 

  info actor;
  actor.name = player.c_str();
  actor.data = actorFile;

  void* actorOffset = bsearch(&actor, (char*) actorFile + sizeof(int), *((int*)actorFile), sizeof(int), &compareActors);
 
  if(actorOffset != NULL){
    updateVector(films, actorOffset);
    return true;
  } 
  return false; 
}

/*
helper function for getCast
it updates the vector and loops over the offsets of the given movie
*/
void imdb::getPeople(vector<string>& players, const void* filmOffset) const {
  char* index = (char*) movieFile + *(int*) filmOffset;
 
  int count = 0;
 
  while(*index != '\0'){  
      index++;
      count++;
  } 
  count+=2;
  index+=2;
  if(count % 2 == 1){
    count++;
    index++;
  }

  count += sizeof(short);
  short castNum = *(short*) index;
 
  index += sizeof(short) + count % 4;
  for(int i = 0; i < castNum; i++) {
  
    int actorFileOffset = *((int*) index + i);
   
    char* person = (char*)actorFile + actorFileOffset;
    string name = "";
    
    while (*person != '\0') {
   
      name += *person;
      person++;
    }
    players.push_back(name);
  }

}


bool imdb::getCast(const film& movie, vector<string>& players) const { 

  info Film = {&movie, movieFile};
  void* filmOffset = bsearch(&Film, (char*) movieFile + sizeof(int), *((int*)movieFile), sizeof(int), &compareFilms);
  if(filmOffset == NULL)
    return false;

  getPeople(players, filmOffset);
  return true;
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
