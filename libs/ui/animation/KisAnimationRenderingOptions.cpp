/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KisAnimationRenderingOptions.h"

#include <QStandardPaths>
#include <QFileInfo>

#include <KisFileUtils.h>

KisAnimationRenderingOptions::KisAnimationRenderingOptions()
    : videoMimeType("video/mp4"),
      frameMimeType("image/png"),
      basename("frame"),
      directory("")
{

}

QString KisAnimationRenderingOptions::resolveAbsoluteDocumentFilePath(const QString &documentPath) const
{
    return
        !documentPath.isEmpty() ?
        documentPath :
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}

QString KisAnimationRenderingOptions::resolveAbsoluteVideoFilePath(const QString &documentPath) const
{
    const QString basePath = resolveAbsoluteDocumentFilePath(documentPath);
    return KritaUtils::resolveAbsoluteFilePath(basePath, videoFileName);
}

QString KisAnimationRenderingOptions::resolveAbsoluteFramesDirectory(const QString &documentPath) const
{
    if (renderMode() == RENDER_VIDEO_ONLY) {
        return QFileInfo(resolveAbsoluteVideoFilePath()).absolutePath();
    }

    const QString basePath = resolveAbsoluteDocumentFilePath(documentPath);
    return KritaUtils::resolveAbsoluteFilePath(basePath, directory);
}

QString KisAnimationRenderingOptions::resolveAbsoluteVideoFilePath() const
{
    return resolveAbsoluteVideoFilePath(lastDocuemntPath);
}

QString KisAnimationRenderingOptions::resolveAbsoluteFramesDirectory() const
{
    return resolveAbsoluteFramesDirectory(lastDocuemntPath);
}

KisAnimationRenderingOptions::RenderMode KisAnimationRenderingOptions::renderMode() const
{
    if (shouldDeleteSequence) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(shouldEncodeVideo);
        return RENDER_VIDEO_ONLY;
    } else if (!shouldEncodeVideo) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!shouldDeleteSequence);
        return RENDER_FRAMES_ONLY;
    } else {
        return RENDER_FRAMES_AND_VIDEO;
    }
}

KisPropertiesConfigurationSP KisAnimationRenderingOptions::toProperties() const
{
    KisPropertiesConfigurationSP config = new KisPropertiesConfiguration();

    config->setProperty("basename", basename);
    config->setProperty("last_document_path", lastDocuemntPath);
    config->setProperty("directory", directory);
    config->setProperty("first_frame", firstFrame);
    config->setProperty("last_frame", lastFrame);
    config->setProperty("sequence_start", sequenceStart);
    config->setProperty("video_mimetype", videoMimeType);
    config->setProperty("frame_mimetype", frameMimeType);

    config->setProperty("encode_video", shouldEncodeVideo);
    config->setProperty("delete_sequence", shouldDeleteSequence);
    config->setProperty("only_unique_frames", wantsOnlyUniqueFrameSequence);

    config->setProperty("ffmpeg_path", ffmpegPath);
    config->setProperty("framerate", frameRate);
    config->setProperty("height", height);
    config->setProperty("width", width);
    config->setProperty("include_audio", includeAudio);
    config->setProperty("filename", videoFileName);
    config->setProperty("custom_ffmpeg_options", customFFMpegOptions);

    config->setPrefixedProperties("frame_export/", frameExportConfig);

    return config;
}

void KisAnimationRenderingOptions::fromProperties(KisPropertiesConfigurationSP config)
{
    basename = config->getPropertyLazy("basename", basename);
    lastDocuemntPath = config->getPropertyLazy("last_document_path", "");
    directory = config->getPropertyLazy("directory", directory);
    firstFrame = config->getPropertyLazy("first_frame", 0);
    lastFrame = config->getPropertyLazy("last_frame", 0);
    sequenceStart = config->getPropertyLazy("sequence_start", 0);
    videoMimeType = config->getPropertyLazy("video_mimetype", videoMimeType);
    frameMimeType = config->getPropertyLazy("frame_mimetype", frameMimeType);

    shouldEncodeVideo = config->getPropertyLazy("encode_video", false);
    shouldDeleteSequence = config->getPropertyLazy("delete_sequence", false);
    wantsOnlyUniqueFrameSequence = config->getPropertyLazy("only_unique_frames", false);

    ffmpegPath = config->getPropertyLazy("ffmpeg_path", "");
    frameRate = config->getPropertyLazy("framerate", 25);
    height = config->getPropertyLazy("height", 0);
    width = config->getPropertyLazy("width", 0);
    includeAudio = config->getPropertyLazy("include_audio", true);
    videoFileName = config->getPropertyLazy("filename", "");
    customFFMpegOptions = config->getPropertyLazy("custom_ffmpeg_options", "");

    frameExportConfig = new KisPropertiesConfiguration();
    frameExportConfig->setPrefixedProperties("frame_export/", frameExportConfig);

}
