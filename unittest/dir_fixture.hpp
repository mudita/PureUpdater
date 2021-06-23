#include <string>
#include <type_traits>
#include <concepts>
#include <filesystem>
#include <stdlib.h>

#ifndef BUILD_DIR
#error Requires build dir to get n dump temp assets
#endif

#ifndef SOURCE_DIR
#error Requires top dir to get assets
#endif

struct Disc
{
    Disc(int meg_size);
    virtual ~Disc();
    std::string drive;
    const int meg_size=1;
    private:
    int umount();
    int clean_dir();
};

struct OneMDisk : public Disc
{
    OneMDisk() : Disc(1) {}
};

struct TwoToEightM : public Disc
{
    TwoToEightM() : Disc(1<<8) {}
};

struct ImageDir 
{
    virtual ~ImageDir() = default;
};

template<class Image, class DDisc>
struct TestAsset
{
    static_assert(std::is_base_of<Disc,DDisc>() == true, "we want disc");
    Image image;
    DDisc disk;
};


struct EmptyImage : ImageDir 
{
    std::string drive;
    std::string tar_name = "sample.tar";
    std::filesystem::path tar_path{std::string(SOURCE_DIR) + "/assets/"};

    EmptyImage();
    virtual ~EmptyImage();
};

/// fixture to copy tar from source to build dir
/// drive to be used as `from` catalog for update
struct StandardImage : public EmptyImage
{
    StandardImage();
};

struct UnpackedImage : public EmptyImage
{
    UnpackedImage();
};

/// image with 3 catalogs and tar is there
struct DeepImage : public EmptyImage
{
    std::string deep_path;
    DeepImage();
};

struct Firmware : public TestAsset<UnpackedImage, TwoToEightM>
{
};

struct TooSmall : public TestAsset<StandardImage, OneMDisk>
{
};

struct AssetEmptyImage : public TestAsset<EmptyImage, OneMDisk>
{
};

struct AverageDisk : public TestAsset<StandardImage, TwoToEightM>
{
};

struct AverageDiskDeepImage : public TestAsset<DeepImage, TwoToEightM>
{};

template<class Image, class DDisc>
struct UnpackAsset
{
    static_assert(std::is_base_of<Disc,DDisc>() == true, "we want disc");
    Image image;
    DDisc disk_os;
    DDisc disk_user;
};

struct UpdateAsset : public UnpackAsset<StandardImage,TwoToEightM>{};

struct UpdateEmptyAsset : public UnpackAsset<EmptyImage,TwoToEightM>{};

struct UpdateSmallAsset : public UnpackAsset<StandardImage,OneMDisk>{};
