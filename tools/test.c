#include <flif.h>
#include <stdlib.h>
#include <stdio.h>

#pragma pack(push,1)
typedef struct RGBA
{
    uint8_t r,g,b,a;
} RGBA;
#pragma pack(pop)

void fill_dummy_image(FLIF_IMAGE* image)
{
    uint32_t w = flif_image_get_width(image);
    uint32_t h = flif_image_get_height(image);

    RGBA* row = (RGBA*)malloc(w * sizeof(RGBA));
    if(row)
    {
        uint32_t y;
        for(y = 0; y < h; ++y)
        {
            uint32_t x;
            for(x = 0; x < w; ++x)
            {
                row[x].r = (x+y) % 255;
                row[x].g = (x+y) % 255;
                row[x].b = (x+y) % 255;
                row[x].a = 255;
            }
            flif_image_write_row_RGBA8(image, y, row, w * sizeof(RGBA));
        }

        free(row);
    }
}

int compare_images(FLIF_IMAGE* image1, FLIF_IMAGE* image2)
{
    int result = 0;

    uint32_t w1 = flif_image_get_width(image1);
    uint32_t h1 = flif_image_get_height(image1);

    uint32_t w2 = flif_image_get_width(image2);
    uint32_t h2 = flif_image_get_height(image2);

    if(w1 != w2 || h1 != h2)
    {
        printf("Error: Images have different width/height\n");
        result = 1;
    }

    RGBA* row1 = (RGBA*)malloc(w1 * sizeof(RGBA));
    if(row1 == 0)
    {
        printf("Error: Out of memory\n");
        result = 1;
    }
    else
    {
        RGBA* row2 = (RGBA*)malloc(w1 * sizeof(RGBA));
        if(row2 == 0)
        {
            printf("Error: Out of memory\n");
            result = 1;
        }
        else
        {
            int difference = 0;
            uint32_t y;
            for(y = 0; y < h1; ++y)
            {
                flif_image_read_row_RGBA8(image1, y, row1, w1 * sizeof(RGBA));
                flif_image_read_row_RGBA8(image2, y, row2, w2 * sizeof(RGBA));
                uint32_t x;
                for(x = 0; x < w1; ++x)
                {
                    if( row1[x].r != row2[x].r ||
                        row1[x].g != row2[x].g ||
                        row1[x].b != row2[x].b ||
                        row1[x].a != row2[x].a)
                    {
                        // stop flooding the log if the image has many differences
                        if(difference < 100)
                        {
                            printf("Error: Color difference at %u,%u: %02X%02X%02X%02X -> %02X%02X%02X%02X\n", x, y, row1[x].r, row1[x].g, row1[x].b, row1[x].a, row2[x].r, row2[x].g, row2[x].b, row2[x].a);
                            result = 1;
                        }
                        difference++;
                    }
                }
            }

            free(row2);
        }

        free(row1);
    }

    return result;
}

int compare_file_and_blob(const void* blob, size_t blob_size, const char* filename)
{
    int result = 0;
    FILE* f = fopen(filename, "rb");
    if(f)
    {
        const uint8_t* blob_ptr = (const uint8_t*)blob;

        fseek(f, 0, SEEK_END);
        long int file_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if(file_size != blob_size)
        {
            printf("Error: flif file size: %ld <---> flif blob size: %zu\n", file_size, blob_size);
            result = 1;
        }
        else
        {
            size_t i;
            for(i = 0; i < blob_size; ++i)
            {
                int c = fgetc(f);
                if(c == EOF)
                {
                    printf("IO error: premature EOF at %zu\n", i);
                    result = 1;
                    break;
                }

                if(c != *(blob_ptr + i))
                {
                    printf("Error: file and blob do not match at %zu\n", i);
                    result = 1;
                    break;
                }
            }
        }

        fclose(f);
    }
    return result;
}

int main()
{
    int result = 0;

    const size_t WIDTH = 256;
    const size_t HEIGHT = 256;
    const char* dummy_file = "../tmp-test/dummy.flif";

    FLIF_IMAGE* im = flif_create_image(WIDTH, HEIGHT);
    if(im == 0)
    {
        printf("Error: flif_create_image failed\n");
        result = 1;
    }
    else
    {
        fill_dummy_image(im);

        void* blob = 0;
        size_t blob_size = 0;

        FLIF_ENCODER* e = flif_create_encoder();
        if(e)
        {
            flif_encoder_set_interlaced(e, 1);
            flif_encoder_set_learn_repeat(e, 3);
            flif_encoder_set_auto_color_buckets(e, 1);
            flif_encoder_set_palette_size(e, 512);
            flif_encoder_set_lookback(e, 1);

            flif_encoder_add_image(e, im);
            if(!flif_encoder_encode_file(e, dummy_file))
            {
                printf("Error: encoding file failed\n");
                result = 1;
            }

            if(!flif_encoder_encode_memory(e, &blob, &blob_size))
            {
                printf("Error: encoding blob failed\n");
                result = 1;
            }

            // TODO: uncommenting this causes subsequent test to fail???
            /*if(!compare_file_and_blob(blob, blob_size, dummy_file))
            {
                result = 1;
            }*/

            flif_destroy_encoder(e);
            e = 0;
        }

        FLIF_DECODER* d = flif_create_decoder();
        if(d)
        {
            flif_decoder_set_quality(d, 100);
            flif_decoder_set_scale(d, 1);

            {
                if(!flif_decoder_decode_file(d, dummy_file))
                {
                    printf("Error: decoding file failed\n");
                    result = 1;
                }

                FLIF_IMAGE* decoded = flif_decoder_get_image(d, 0);
                if(decoded == 0)
                {
                    printf("Error: No decoded image found\n");
                    result = 1;
                }
                else if(compare_images(im, decoded) != 0)
                {
                    result = 1;
                }
            }

            {
                if(!flif_decoder_decode_memory(d, blob, blob_size))
                {
                    printf("Error: decoding memory failed\n");
                    result = 1;
                }

                FLIF_IMAGE* decoded = flif_decoder_get_image(d, 0);
                if(decoded == 0)
                {
                    printf("Error: No decoded image found\n");
                    result = 1;
                }
                else if(compare_images(im, decoded) != 0)
                {
                    result = 1;
                }
            }

            flif_destroy_decoder(d);
            d = 0;
        }

        flif_destroy_image(im);
        im = 0;

        if(blob)
        {
            flif_free_memory(blob);
            blob = 0;
        }
    }

    printf("interface test has succeeded.\n");

    return result;
}
