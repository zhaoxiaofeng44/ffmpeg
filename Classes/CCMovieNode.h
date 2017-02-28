#ifndef __MovieNode_H__
#define __MovieNode_H__

#include "cocos2d.h"

//#include "core/CVMacros.h"
//#include "core/CVDispatcher.h"


struct ffmpeg_movie_context;
struct ffmpeg_video_context;

enum class MovieState
{
	MOVIE_PAUSE,
	MOVIE_PLAYING
};

enum MovieEvent
{
	PLAYING_FINISH
};

NS_CC_BEGIN

class MovieNode : public Sprite
{
	//USE_EVENT_DISPATCHER

public:
    virtual ~MovieNode();

	MovieNode();

	bool init(const std::string &pszFileName);

	static MovieNode *create(const std::string &pszFileName);

    void play(void);

    void pause(void);

	virtual void update(float dt) override;

	virtual void draw(Renderer *renderer, const kmMat4& transform, bool transformUpdated) override;

protected:
   
	MovieState mMovieState;

	ffmpeg_movie_context *ffmpegMovieCtx;
	ffmpeg_video_context *ffmpegVideoCtx;

	V3F_C4B_T2F_Quad _movieCustomQuad;
    CustomCommand _movieCustomCommand;          /// quad command

	void drawSprite(Renderer *renderer, const kmMat4& transform);

	void convertToWorldCoordinates(V3F_C4B_T2F_Quad* quads, ssize_t quantity, const kmMat4& modelView);
};

NS_CC_END

#endif
