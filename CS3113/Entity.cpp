#include "Entity.h"

Entity::Entity() :
    mPosition{ 0.0f, 0.0f }, mMovement{ 0.0f, 0.0f },
    mVelocity{ 0.0f, 0.0f }, mAcceleration{ 0.0f, 0.0f },
    mScale{ DEFAULT_SIZE, DEFAULT_SIZE },
    mColliderDimensions{ DEFAULT_SIZE, DEFAULT_SIZE },
    mTexture{}, mTextureType{ SINGLE }, mSpriteSheetDimensions{},
    mAnimationAtlas{}, mAnimationIndices{}, mDirection{ RIGHT },
    mFrameSpeed{ 0 }, mSpeed{ DEFAULT_SPEED }, mAngle{ 0.0f } {}

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath) :
    mPosition{ position }, mMovement{ 0.0f, 0.0f },
    mVelocity{ 0.0f, 0.0f }, mAcceleration{ 0.0f, 0.0f },
    mScale{ scale }, mColliderDimensions{ scale },
    mTexture{ LoadTexture(textureFilepath) }, mTextureType{ SINGLE },
    mSpriteSheetDimensions{}, mAnimationAtlas{}, mAnimationIndices{},
    mDirection{ RIGHT }, mFrameSpeed{ 0 }, mSpeed{ DEFAULT_SPEED },
    mAngle{ 0.0f } {}

Entity::Entity(
    Vector2                               position,
    Vector2                               scale,
    const char                           *textureFilepath,
    TextureType                           textureType,
    Vector2                               spriteSheetDimensions,
    std::map<Direction, std::vector<int>> animationAtlas
) :
    mPosition{ position }, mMovement{ 0.0f, 0.0f },
    mVelocity{ 0.0f, 0.0f }, mAcceleration{ 0.0f, 0.0f },
    mScale{ scale }, mColliderDimensions{ scale },
    mTexture{ LoadTexture(textureFilepath) }, mTextureType{ textureType },
    mSpriteSheetDimensions{ spriteSheetDimensions },
    mAnimationAtlas{ animationAtlas },
    mAnimationIndices{ animationAtlas.at(RIGHT) }, mDirection{ RIGHT },
    mFrameSpeed{ DEFAULT_FRAME_SPEED }, mSpeed{ DEFAULT_SPEED },
    mAngle{ 0.0f } {}

Entity::~Entity() { UnloadTexture(mTexture); }

void Entity::resetCollisionFlags()
{
    mIsCollidingTop      = false;
    mIsCollidingBottom   = false;
    mIsCollidingRight    = false;
    mIsCollidingLeft     = false;
    mMissionFailed       = false;
    mMissionAccomplished = false;
}

bool Entity::isColliding(Entity *other) const
{
    float xDistance = fabs(mPosition.x - other->mPosition.x) -
        ((mColliderDimensions.x + other->mColliderDimensions.x) / 2.0f);
    float yDistance = fabs(mPosition.y - other->mPosition.y) -
        ((mColliderDimensions.y + other->mColliderDimensions.y) / 2.0f);

    return xDistance < 0.0f && yDistance < 0.0f;
}

void Entity::checkCollisionY(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];

        if (isColliding(collidableEntity))
        {
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(
                yDistance
                - (mColliderDimensions.y / 2.0f)
                - (collidableEntity->mColliderDimensions.y / 2.0f)
            );

            if (mVelocity.y > 0.0f)
            {
                mPosition.y       -= yOverlap;
                mVelocity.y        = 0.0f;
                mIsCollidingBottom = true;

                if (collidableEntity->mEntityType == WIN_PLATFORM)
                    mMissionAccomplished = true;
                else if (collidableEntity->mEntityType == FORBIDDEN_AREA)
                    mMissionFailed = true;
            }
            else if (mVelocity.y < 0.0f)
            {
                mPosition.y    += yOverlap;
                mVelocity.y     = 0.0f;
                mIsCollidingTop = true;
                mMissionFailed  = true;
            }
        }
    }
}

void Entity::checkCollisionX(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];

        if (isColliding(collidableEntity))
        {
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(
                yDistance
                - (mColliderDimensions.y / 2.0f)
                - (collidableEntity->mColliderDimensions.y / 2.0f)
            );

            if (yOverlap < Y_COLLISION_THRESHOLD) continue;

            float xDistance = fabs(mPosition.x - collidableEntity->mPosition.x);
            float xOverlap  = fabs(
                xDistance
                - (mColliderDimensions.x / 2.0f)
                - (collidableEntity->mColliderDimensions.x / 2.0f)
            );

            if (mVelocity.x > 0.0f)
            {
                mPosition.x      -= xOverlap;
                mVelocity.x       = 0.0f;
                mIsCollidingRight = true;
            }
            else if (mVelocity.x < 0.0f)
            {
                mPosition.x     += xOverlap;
                mVelocity.x      = 0.0f;
                mIsCollidingLeft = true;
            }

            mMissionFailed = true;
        }
    }
}

/**
 * Updates the current frame index of an entity's animation based on the
 * elapsed time and frame speed.
 *
 * @param deltaTime represents the time elapsed since the last frame update.
 */
void Entity::animate(float deltaTime)
{
    mAnimationIndices = mAnimationAtlas.at(mDirection);

    mAnimationTime += deltaTime;
    float secondsPerFrame = 1.0f / mFrameSpeed;

    if (mAnimationTime >= secondsPerFrame)
    {
        mAnimationTime = 0.0f;

        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mAnimationIndices.size();
    }
}

void Entity::update(
    float deltaTime,
    Entity *collidableEntities,
    int collisionCheckCount
)
{
    resetCollisionFlags();

    if (mMovement.x != 0.0f)
    {
        mVelocity.x = mMovement.x * mSpeed;
    }
    else
    {
        float previousVelocityX = mVelocity.x;
        mVelocity.x += mAcceleration.x * deltaTime;

        if ((previousVelocityX > 0.0f && mVelocity.x < 0.0f) ||
            (previousVelocityX < 0.0f && mVelocity.x > 0.0f))
        {
            mVelocity.x     = 0.0f;
            mAcceleration.x = 0.0f;
        }
    }

    mVelocity.y += mAcceleration.y * deltaTime;

    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(collidableEntities, collisionCheckCount);

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(collidableEntities, collisionCheckCount);

    if (mTextureType == ATLAS)
    {
        if (mVelocity.y > 0.0f)
        {
            animate(deltaTime);
        }
        else
        {
            mCurrentFrameIndex = 0;
            mAnimationTime     = 0.0f;
        }
    }
}

void Entity::render()
{
    Rectangle textureArea;

    switch (mTextureType)
    {
    case SINGLE:
        textureArea = {
            0.0f, 0.0f,
            static_cast<float>(mTexture.width),
            static_cast<float>(mTexture.height)
        };
        break;
    case ATLAS:
        textureArea = getUVRectangle(
            &mTexture,
            mAnimationIndices[mCurrentFrameIndex],
            mSpriteSheetDimensions.x,
            mSpriteSheetDimensions.y
        );
        break;
    default:
        break;
    }

    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    Vector2 originOffset = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    DrawTexturePro(
        mTexture,
        textureArea, destinationArea, originOffset,
        mAngle, WHITE
    );
}
