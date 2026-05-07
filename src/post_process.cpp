#include "post_process.h"
#include "shaders.h"

void PostProcessor::init(int w, int h) {
    width = w;
    height = h;

    quad.init();

    brightShader.init(Shaders::COMPOSITE_VERT, Shaders::BLOOM_BRIGHT_FRAG);
    blurShader.init(Shaders::COMPOSITE_VERT, Shaders::BLOOM_BLUR_FRAG);
    compositeShader.init(Shaders::COMPOSITE_VERT, Shaders::COMPOSITE_FRAG);

    int bw = w / 2, bh = h / 2;
    brightFBO.create(bw, bh);
    pingFBO.create(bw, bh);
    pongFBO.create(bw, bh);
    outputFBO.create(w, h);
}

void PostProcessor::resize(int w, int h) {
    destroy();
    init(w, h);
}

void PostProcessor::process(GLuint sceneTexture, const PostProcessSettings& settings) {
    extractBright(sceneTexture, settings.bloomThreshold);
    blurPass(settings.bloomPasses);
    composite(sceneTexture, settings);
}

void PostProcessor::extractBright(GLuint sceneTexture, float threshold) {
    brightFBO.bind();
    glClear(GL_COLOR_BUFFER_BIT);

    brightShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    brightShader.setInt("uTexture", 0);
    brightShader.setFloat("uThreshold", threshold);

    quad.renderQuad();
    brightFBO.unbind();
}

void PostProcessor::blurPass(int passes) {
    bool horizontal = true;
    bool firstPass = true;

    blurShader.use();
    blurShader.setInt("uTexture", 0);

    for (int i = 0; i < passes * 2; i++) {
        if (horizontal) {
            pingFBO.bind();
        } else {
            pongFBO.bind();
        }
        glClear(GL_COLOR_BUFFER_BIT);

        blurShader.setBool("uHorizontal", horizontal);

        int bw = width / 2;
        int bh = height / 2;
        float texelSize = horizontal ? (1.0f / bw) : (1.0f / bh);
        blurShader.setFloat("uTexelSize", texelSize);

        glActiveTexture(GL_TEXTURE0);
        if (firstPass) {
            glBindTexture(GL_TEXTURE_2D, brightFBO.colorTexture);
            firstPass = false;
        } else {
            glBindTexture(GL_TEXTURE_2D, horizontal ? pongFBO.colorTexture : pingFBO.colorTexture);
        }

        quad.renderQuad();

        if (horizontal) pingFBO.unbind();
        else pongFBO.unbind();

        horizontal = !horizontal;
    }
}

void PostProcessor::composite(GLuint sceneTexture, const PostProcessSettings& settings) {
    outputFBO.bind();
    glClear(GL_COLOR_BUFFER_BIT);

    compositeShader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    compositeShader.setInt("uScene", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingFBO.colorTexture);
    compositeShader.setInt("uBloom", 1);

    compositeShader.setFloat("uBloomIntensity", settings.bloomIntensity);
    compositeShader.setFloat("uExposure", settings.exposure + settings.lightningFlash * 2.0f);
    compositeShader.setFloat("uContrast", settings.contrast);
    compositeShader.setFloat("uSaturation", settings.saturation);
    compositeShader.setVec3("uTint", settings.tint);
    compositeShader.setFloat("uVignetteStrength", settings.vignetteStrength);

    quad.renderQuad();
    outputFBO.unbind();
}

GLuint PostProcessor::getOutputTexture() const {
    return outputFBO.colorTexture;
}

void PostProcessor::destroy() {
    brightFBO.destroy();
    pingFBO.destroy();
    pongFBO.destroy();
    outputFBO.destroy();
    brightShader.destroy();
    blurShader.destroy();
    compositeShader.destroy();
    quad.destroy();
}
