#pragma once
#include <glm/glm.hpp>
#include "uniforms.h"
#include "fragment.h"
#include "color.h"
#include <random>
#include "print.h"
#include "FastNoiseLite.h"




int selectedPlanet =6;

Vertex vertexShader(const Vertex& vertex, const Uniforms& uniforms) {
    // Apply transformations to the input vertex using the matrices from the uniforms
    glm::vec4 clipSpaceVertex = uniforms.projection * uniforms.view * uniforms.model * glm::vec4(vertex.position, 1.0f);

    // Perspective divide
    glm::vec3 ndcVertex = glm::vec3(clipSpaceVertex) / clipSpaceVertex.w;

    // Apply the viewport transform
    glm::vec4 screenVertex = uniforms.viewport * glm::vec4(ndcVertex, 1.0f);
    
    // Transform the normal
    glm::vec3 transformedNormal = glm::mat3(uniforms.model) * vertex.normal;
    transformedNormal = glm::normalize(transformedNormal);

    // Return the transformed vertex as a vec3
    return Vertex{
        glm::vec3(screenVertex),
        transformedNormal,
        vertex.position,
        // vertex.normal, // non transformed normal
    };
}


// Planeta 1: Gaseoso
Fragment fragmentShader(const Fragment& fragment) {

if (selectedPlanet == 1) {
    Color color;

    // Define los colores base para el planeta Omicron
    glm::vec3 baseColor = glm::vec3(0.2f, 0.3f, 0.8f); // Azul
    glm::vec3 landColor = glm::vec3(0.5f, 0.7f, 0.3f); // Verde
    glm::vec3 cloudColor = glm::vec3(1.0f, 1.0f, 1.0f); // Blanco

    glm::vec2 uv = glm::vec2(fragment.original.x, fragment.original.y);

    // Simulación de terreno
    float elevation = sin(uv.x * 10.0f) * cos(uv.y * 10.0f); /* Calcular la elevación basada en coordenadas uv */;

    // Ajusta la apariencia del planeta Omicron basado en la elevación
    if (elevation < 0.1f) {
        // Color del agua
        color = Color(baseColor.x, baseColor.y, baseColor.z);
    } else if (elevation < 0.5f) {
        // Color de la tierra
        color = Color(landColor.x, landColor.y, landColor.z);
    } else {
        // Color de las nubes
        color = Color(cloudColor.x, cloudColor.y, cloudColor.z); 
    }

    Fragment processedFragment = fragment;
    processedFragment.color = color * fragment.intensity;

    return processedFragment;
}

if (selectedPlanet == 2) {
    Color color;

    // Define los colores base y los colores de las capas
    glm::vec3 baseColor = glm::vec3(0.02f, 0.23f, 0.35f); // #023859
    glm::vec3 secondColor = glm::vec3(0.03f, 0.35f, 0.55f); // #07598C
    glm::vec3 thirdColor = glm::vec3(0.65f, 0.39f, 0.08f); // #A66414
    glm::vec3 fourthColor = glm::vec3(0.35f, 0.24f, 0.15f); // #593E25

    glm::vec2 uv = glm::vec2(fragment.original.x, fragment.original.y);

    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin); // Cambiado el tipo de ruido a Perlin

    // Ajusta estos valores para controlar la apariencia de las capas
    float baseLayerThreshold = 10.4f;
    float secondLayerThreshold = 0.7f;

    float ox = 1200.0f;
    float oy = 3000.0f;
    float zoom = 200.0f;

    float noiseValue = noiseGenerator.GetNoise((uv.x + ox) * zoom, (uv.y + oy) * zoom);

    glm::vec3 tmpColor = baseColor;

    // Define la frecuencia y amplitud de las ondas de medusa
    float frequencyX = 100.0f;
    float frequencyY = 100.0f;
    float amplitude = 0.2f;

    // Simula una animación en el "iris" utilizando un temporizador simple
    static float t = 0.0f;
    t += 0.01f; // Ajusta la velocidad de la animación cambiando este valor

    // Calcula un valor de onda utilizando la función seno en ambos ejes
    float waveValue = (sin(uv.x * frequencyX + t) * sin(uv.y * frequencyY + t) * 0.5f + 0.5f);

    if (waveValue > amplitude) {
        // Asigna el color base
        tmpColor = baseColor;
    } else {
        // Asigna el segundo color
        tmpColor = secondColor;
    }

    //-------------------------------------

    // Agregar múltiples manchas de medusa
    glm::vec2 medusaCenters[] = {
        glm::vec2(0.3f, 0.6f), glm::vec2(0.7f, 0.4f), glm::vec2(0.5f, 0.5f),
        glm::vec2(0.2f, 0.8f), glm::vec2(0.8f, 0.2f), glm::vec2(0.3f, 0.3f),
        glm::vec2(0.7f, 0.7f), glm::vec2(0.4f, 0.6f), glm::vec2(0.9f, 1.3f),
        glm::vec2(0.3f, 0.2f), glm::vec2(0.4f, 4.4f), glm::vec2(0.4f, 3.4f),
        glm::vec2(0.6f, 0.1f), glm::vec2(0.4f, 8.4f) // Añade más coordenadas para más manchas
    };
    float medusaRadius = 0.05f;

    for (int i = 0; i < 3; i++) {
        float distanceToMedusa = glm::distance(uv, medusaCenters[i]);

        if (distanceToMedusa < medusaRadius) {
            float opacity = 10.5f;
            tmpColor = glm::mix(tmpColor, thirdColor, opacity);
        }        
    }

    // Puedes ajustar estos valores según tus preferencias para la apariencia de las capas
    float cloudLayerThreshold = 0.5f;
    float cloudDensity = 0.7f;

    float oxc = 5500.0f;
    float oyc = 6900.0f;
    float zoomc = 300.0f;

    float noiseValueC = noiseGenerator.GetNoise(sin(((uv.x + oxc) * zoomc)), sin((uv.y + oyc) * zoomc));

    if (noiseValueC < cloudLayerThreshold) {
        // Agrega ruido de nubes utilizando el cuarto color
        tmpColor = glm::mix(tmpColor, fourthColor, cloudDensity);
    }

    color = Color(tmpColor.x, tmpColor.y, tmpColor.z);

    Fragment processedFragment = fragment;
    processedFragment.color = (color * fragment.intensity * zoomc);

    return processedFragment;
}

if (selectedPlanet == 3) {
    Color color;

    // Define el color base para todo el planeta
    glm::vec3 baseColor = glm::vec3(0.95f, 0.76f, 0.21f); // #F2C335

    // Colores de las bacterias
    glm::vec3 secondColor = glm::vec3(0.95f, 0.45f, 0.02f); // #F27405
    glm::vec3 thirdColor = glm::vec3(0.85f, 0.24f, 0.02f); // #D93D04
    glm::vec3 fourthColor = glm::vec3(0.55f, 0.01f, 0.01f); // #8C0303

    glm::vec2 uv = glm::vec2(fragment.original.x, fragment.original.y);

    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    // Ajusta la escala del ruido para controlar la distribución de las bacterias
    float noiseScale = 7.10f;
    float noiseValue = noiseGenerator.GetNoise(uv.x * noiseScale, uv.y * noiseScale * noiseScale );

    // Define un umbral para separar las áreas con bacterias y sin bacterias
    float bacteriaThreshold = 0.3f;

    if (noiseValue > bacteriaThreshold) {
        // Asigna uno de los colores de las bacterias de manera aleatoria
        glm::vec3 bacteriaColors[] = {baseColor, secondColor, thirdColor, fourthColor};
        int colorIndex = int(noiseValue * 900.0f) % 4;
        glm::vec3 tmpColor = bacteriaColors[colorIndex];

        // Aplica el color uniforme a la intensidad del color de las bacterias
        tmpColor *= baseColor;

        color = Color(tmpColor.x, tmpColor.y, tmpColor.z);
    } 
    else {
        // Asigna un color de fondo
        // Asigna uno de los colores de las bacterias de manera aleatoria
        glm::vec3 bacteriaColors[] = {baseColor, secondColor, thirdColor, fourthColor};
        int colorIndex = int(noiseValue * 900.0f) % 4;
        glm::vec3 tmpColor = bacteriaColors[colorIndex];

        // Aplica el color uniforme a la intensidad del color de las bacterias
        tmpColor *= baseColor;

        color = Color(tmpColor.x, tmpColor.y, tmpColor.z);
    }

    Fragment processedFragment = fragment;
    processedFragment.color = color * fragment.intensity;

    return processedFragment;
}

if (selectedPlanet == 4) {
    Color color;

    // Define los cuatro colores para las regiones del cerebro
    glm::vec3 colorRegion1 = glm::vec3(0.65f, 0.46f, 0.40f); // #A67665
    glm::vec3 colorRegion2 = glm::vec3(0.85f, 0.65f, 0.59f); // #D9A796
    glm::vec3 colorRegion3 = glm::vec3(0.25f, 0.05f, 0.0f);  // #400D01
    glm::vec3 colorRegion4 = glm::vec3(0.45f, 0.21f, 0.16f); // #733729

    // Simula el aspecto del cerebro utilizando ruido y ondas
    glm::vec2 uv = glm::vec2(fragment.original.x, fragment.original.y);

    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin); // Cambiado el tipo de ruido a Perlin

    // Ajusta la escala del ruido para controlar la distribución de las regiones
    float noiseScale = 10000.0f;
    float noiseValue = noiseGenerator.GetNoise(uv.x * noiseScale, uv.y * noiseScale);

    // Define la frecuencia y amplitud de las ondas
    float waveFrequency = 100.0f;
    float waveAmplitude = 0.5f;

    // Calcula un valor de onda utilizando la función seno en ambos ejes
    float waveValue = (sin(uv.x * waveFrequency) * sin(uv.y * waveFrequency) * waveAmplitude);

    // Combina el valor de ruido con el valor de onda para agregar textura
    float finalValue = noiseValue + waveValue ;

    // Define umbrales para las regiones del cerebro
    float threshold1 = 0.2f;
    float threshold2 = 0.7f;
    float threshold3 = 0.1f;
    float threshold4 = 0.1f;

    // Asigna colores a las regiones según el valor combinado de ruido y onda
    if (finalValue < threshold1) {
        color = Color(colorRegion1.x, colorRegion1.y, colorRegion1.z);
    } else if (finalValue < threshold2) {
        color = Color(colorRegion2.x, colorRegion2.y, colorRegion2.z);
    } else if (finalValue < threshold3) {
        color = Color(colorRegion3.x, colorRegion3.y, colorRegion3.z);
    } else {
        color = Color(colorRegion4.x, colorRegion4.y, colorRegion4.z);
    }

    Fragment processedFragment = fragment;
    processedFragment.color = color;

    return processedFragment;
}

if (selectedPlanet == 5) {
        Color color;

        // Define los colores base para el planeta "Cartílago Hialino"
        glm::vec3 baseColor = glm::vec3(0.686f, 0.443f, 0.851f); // #AF71D9 (Color predominante)
        glm::vec3 color1 = glm::vec3(0.768f, 0.796f, 0.949f);     // #C4CBF2 (Color secundario)
        glm::vec3 color2 = glm::vec3(0.018f, 0.615f, 0.850f);     // #049DD9 (Color secundario)
        glm::vec3 color3 = glm::vec3(0.018f, 0.698f, 0.850f);     // #04B2D9 (Color secundario)
        glm::vec3 color4 = glm::vec3(0.023f, 0.859f, 0.949f);     // #05DBF2 (Color secundario)

        glm::vec2 uv = glm::vec2(fragment.original.x, fragment.original.y);

        // Simula un patrón en el planeta "Cartílago Hialino" basado en coordenadas uv
        float pattern = sin(uv.x * 50.0f) * cos(uv.y * 50.0f);

        // Ajusta la escala y la apariencia del patrón
        float cellSize = 1.105f; // Tamaño de las células
        pattern = glm::fract(pattern / cellSize); // Divide el patrón en celdas más pequeñas

        // Ajusta la apariencia del planeta basada en el patrón
        if (pattern < 0.1f) {
            color = Color(baseColor.x, baseColor.y, baseColor.z);
        } else if (pattern < 0.3f) {
            color = Color(color1.x, color1.y, color1.z);
        } else if (pattern < 0.5f) {
            color = Color(color2.x, color2.y, color2.z);
        } else if (pattern < 0.7f) {
            color = Color(color3.x, color3.y, color3.z);
        } else {
            color = Color(color4.x, color4.y, color4.z);
        }

        Fragment processedFragment = fragment;
        processedFragment.color = color * fragment.intensity;

        return processedFragment;
    }

    if (selectedPlanet == 6) {
    Color color;

    glm::vec3 secondColor = glm::vec3(0.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f);
    glm::vec3 mainColor = glm::vec3(255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f);

    glm::vec2 uv = glm::vec2(fragment.original.x, fragment.original.y);

    FastNoiseLite noiseGenerator;
    noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

    float offsetX = 8000.0f;
    float offsetY = 1000.0f;
    float scale = 30000.0f;

    // Genera el valor de ruido
    float noiseValue = noiseGenerator.GetNoise((uv.x + offsetX) * scale, (uv.y + offsetY) * scale);
    noiseValue = (noiseValue + 1.0f) * 0.9f;

    // Interpola entre el color base y el color secundario basado en el valor de ruido
    secondColor = glm::mix(mainColor, secondColor, noiseValue);

    if (noiseValue > 0.99f) {
        // Calcula el valor sinusoide para crear líneas
        float sinValue = glm::cos(uv.y * 100.01f) * 0.1f;

        sinValue = glm::smoothstep(100.8f, 10.0f, sinValue);

        // Combina el color base con las líneas sinusoide
        secondColor = secondColor + glm::vec3(sinValue);
    }

    // Interpola entre el color base y el color secundario basado en el valor de ruido
    mainColor = glm::mix(mainColor, mainColor, noiseValue);

    // Interpola entre el color base y el color secundario basado en el valor de ruido
    secondColor = glm::mix(mainColor, secondColor, noiseValue);

    color = Color(secondColor.x, secondColor.y, secondColor.z);

    Fragment processedFragment = fragment;
    processedFragment.color = color * fragment.intensity * fragment.intensity * fragment.intensity;

    return processedFragment;
}
}
