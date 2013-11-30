/*
See README for information on this file.
*/

#include <limits>
#include <iostream>
#include <gd.h>
#include <sys/mman.h>

#define OSMIUM_WITH_PBF_INPUT
#define OSMIUM_WITH_XML_INPUT

#include <osmium.hpp>

#define GRID_SQUARES_PER_DEGREE 50

/* ================================================== */

typedef uint16_t node_count_t;

class NodeDensityHandler : public Osmium::Handler::Base {

    node_count_t* m_node_count;
    int m_xsize;
    int m_ysize;
    node_count_t m_max_count;
    const char *m_indexfilename;
    const char *m_pngfilename;

public:

    NodeDensityHandler(const char *indexfilename, const char *pngfilename)
        : Base(),
          m_xsize(360 * GRID_SQUARES_PER_DEGREE),
          m_ysize(180 * GRID_SQUARES_PER_DEGREE),
          m_indexfilename(indexfilename),
          m_pngfilename(pngfilename) 
    {
        m_node_count = static_cast<node_count_t*>(calloc(m_xsize * m_ysize, sizeof(node_count_t)));
        memset(static_cast<void *>(m_node_count), 0, m_xsize * m_ysize * sizeof(node_count_t));
    }

    ~NodeDensityHandler() 
    {
        free(m_node_count);
    }

    void node(const shared_ptr<Osmium::OSM::Node const>& node) 
    {
        int x = int((180 + node->position().lon()) * GRID_SQUARES_PER_DEGREE);
        int y = int(( 90 - node->position().lat()) * GRID_SQUARES_PER_DEGREE);
        if (x <        0) x =         0;
        if (x >= m_xsize) x = m_xsize-1;
        if (y <        0) y =         0;
        if (y >= m_ysize) y = m_ysize-1;
        int n = y * m_xsize + x;
        if (m_node_count[n] < std::numeric_limits<node_count_t>::max() - 1) {
            m_node_count[n]++;
        }
        if (m_node_count[n] > m_max_count) {
            m_max_count = m_node_count[n];
        }
    }

    void after_nodes() 
    {
        std::cerr << "max_count=" << m_max_count << "\n";

        int fd = open(m_indexfilename, O_RDWR | O_CREAT, 0600);

        if (fd < 0) 
        {
            throw std::bad_alloc();
        }

        if (ftruncate(fd, m_xsize * m_ysize) < 0) 
        {
            throw std::bad_alloc();
        }

        uint8_t *index = static_cast<uint8_t *>(mmap(NULL, 8 + m_xsize* m_ysize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        if (index == MAP_FAILED) 
        {
            throw std::bad_alloc();
        }

        // fill first 8 bytes with version string
        memcpy(static_cast<void *>(index), static_cast<const void *>("NDNS1000"), 8);

        uint8_t *array = index+8;

        for (int y=0; y < m_ysize; y++)
        {
            for (int x=0; x < m_xsize; x++)
            {
                int n = y * m_xsize + x;
                if (m_node_count[n] > 41019)
                {
                    array[n]=255;
                }
                else
                {
                    // this results in values of (approx)
                    // 100 nodes -> 35
                    // 200 nodes -> 53
                    // 1000 nodes -> 109
                    // 2000 nodes -> 135
                    // 41019 nodes (and above) -> 255
                    array[n] = (log(m_node_count[n]+70)-log(70)) * 40;
                }
            }
        }

        if (!m_pngfilename) return;

        gdImagePtr im = gdImageCreate(m_xsize, m_ysize);

        // heatmap colours from gheat
        gdImageColorAllocate(im,53,52,61);
        gdImageColorAllocate(im,51,51,61);
        gdImageColorAllocate(im,49,50,62);
        gdImageColorAllocate(im,48,48,63);
        gdImageColorAllocate(im,46,46,64);
        gdImageColorAllocate(im,44,44,65);
        gdImageColorAllocate(im,42,42,65);
        gdImageColorAllocate(im,40,40,67);
        gdImageColorAllocate(im,37,37,67);
        gdImageColorAllocate(im,35,35,68);
        gdImageColorAllocate(im,32,32,69);
        gdImageColorAllocate(im,29,29,70);
        gdImageColorAllocate(im,26,26,69);
        gdImageColorAllocate(im,24,24,71);
        gdImageColorAllocate(im,21,21,72);
        gdImageColorAllocate(im,19,19,73);
        gdImageColorAllocate(im,16,16,74);
        gdImageColorAllocate(im,14,14,76);
        gdImageColorAllocate(im,12,12,77);
        gdImageColorAllocate(im,10,10,77);
        gdImageColorAllocate(im,8,8,77);
        gdImageColorAllocate(im,7,7,79);
        gdImageColorAllocate(im,6,6,82);
        gdImageColorAllocate(im,5,5,85);
        gdImageColorAllocate(im,4,5,89);
        gdImageColorAllocate(im,3,4,92);
        gdImageColorAllocate(im,2,4,97);
        gdImageColorAllocate(im,1,5,101);
        gdImageColorAllocate(im,1,6,106);
        gdImageColorAllocate(im,1,8,110);
        gdImageColorAllocate(im,0,9,115);
        gdImageColorAllocate(im,0,12,120);
        gdImageColorAllocate(im,0,15,126);
        gdImageColorAllocate(im,0,18,131);
        gdImageColorAllocate(im,0,22,136);
        gdImageColorAllocate(im,0,26,142);
        gdImageColorAllocate(im,0,30,147);
        gdImageColorAllocate(im,0,35,152);
        gdImageColorAllocate(im,0,40,157);
        gdImageColorAllocate(im,0,46,162);
        gdImageColorAllocate(im,0,52,167);
        gdImageColorAllocate(im,0,58,171);
        gdImageColorAllocate(im,0,65,175);
        gdImageColorAllocate(im,0,71,180);
        gdImageColorAllocate(im,0,78,184);
        gdImageColorAllocate(im,0,86,189);
        gdImageColorAllocate(im,0,93,193);
        gdImageColorAllocate(im,0,99,196);
        gdImageColorAllocate(im,0,107,199);
        gdImageColorAllocate(im,0,112,203);
        gdImageColorAllocate(im,0,117,205);
        gdImageColorAllocate(im,0,122,208);
        gdImageColorAllocate(im,0,130,213);
        gdImageColorAllocate(im,0,139,218);
        gdImageColorAllocate(im,0,147,222);
        gdImageColorAllocate(im,0,156,228);
        gdImageColorAllocate(im,0,164,232);
        gdImageColorAllocate(im,0,173,236);
        gdImageColorAllocate(im,0,180,240);
        gdImageColorAllocate(im,0,188,244);
        gdImageColorAllocate(im,0,195,247);
        gdImageColorAllocate(im,0,202,250);
        gdImageColorAllocate(im,0,208,252);
        gdImageColorAllocate(im,0,214,254);
        gdImageColorAllocate(im,0,219,255);
        gdImageColorAllocate(im,0,224,255);
        gdImageColorAllocate(im,0,228,255);
        gdImageColorAllocate(im,0,232,255);
        gdImageColorAllocate(im,0,234,255);
        gdImageColorAllocate(im,0,234,254);
        gdImageColorAllocate(im,0,234,251);
        gdImageColorAllocate(im,0,234,248);
        gdImageColorAllocate(im,0,234,246);
        gdImageColorAllocate(im,0,234,242);
        gdImageColorAllocate(im,0,234,238);
        gdImageColorAllocate(im,0,234,234);
        gdImageColorAllocate(im,0,234,230);
        gdImageColorAllocate(im,0,234,225);
        gdImageColorAllocate(im,0,234,220);
        gdImageColorAllocate(im,0,234,214);
        gdImageColorAllocate(im,0,233,209);
        gdImageColorAllocate(im,0,231,203);
        gdImageColorAllocate(im,0,229,197);
        gdImageColorAllocate(im,0,227,190);
        gdImageColorAllocate(im,0,224,184);
        gdImageColorAllocate(im,0,222,178);
        gdImageColorAllocate(im,0,219,171);
        gdImageColorAllocate(im,0,216,165);
        gdImageColorAllocate(im,0,215,160);
        gdImageColorAllocate(im,0,213,156);
        gdImageColorAllocate(im,0,211,151);
        gdImageColorAllocate(im,0,209,147);
        gdImageColorAllocate(im,0,207,143);
        gdImageColorAllocate(im,0,205,138);
        gdImageColorAllocate(im,0,203,134);
        gdImageColorAllocate(im,0,201,129);
        gdImageColorAllocate(im,0,200,125);
        gdImageColorAllocate(im,0,198,120);
        gdImageColorAllocate(im,0,196,116);
        gdImageColorAllocate(im,0,194,112);
        gdImageColorAllocate(im,0,193,108);
        gdImageColorAllocate(im,0,191,104);
        gdImageColorAllocate(im,0,190,99);
        gdImageColorAllocate(im,0,188,95);
        gdImageColorAllocate(im,0,187,92);
        gdImageColorAllocate(im,0,186,88);
        gdImageColorAllocate(im,0,184,84);
        gdImageColorAllocate(im,0,184,80);
        gdImageColorAllocate(im,0,183,77);
        gdImageColorAllocate(im,0,182,74);
        gdImageColorAllocate(im,0,182,70);
        gdImageColorAllocate(im,0,181,68);
        gdImageColorAllocate(im,0,180,65);
        gdImageColorAllocate(im,0,180,62);
        gdImageColorAllocate(im,0,180,60);
        gdImageColorAllocate(im,1,180,57);
        gdImageColorAllocate(im,3,180,55);
        gdImageColorAllocate(im,6,181,53);
        gdImageColorAllocate(im,9,181,51);
        gdImageColorAllocate(im,12,182,48);
        gdImageColorAllocate(im,15,183,46);
        gdImageColorAllocate(im,19,184,44);
        gdImageColorAllocate(im,23,185,43);
        gdImageColorAllocate(im,27,187,40);
        gdImageColorAllocate(im,31,188,39);
        gdImageColorAllocate(im,36,190,37);
        gdImageColorAllocate(im,40,191,35);
        gdImageColorAllocate(im,45,194,34);
        gdImageColorAllocate(im,50,196,33);
        gdImageColorAllocate(im,55,198,31);
        gdImageColorAllocate(im,60,200,30);
        gdImageColorAllocate(im,66,202,28);
        gdImageColorAllocate(im,71,204,26);
        gdImageColorAllocate(im,76,207,25);
        gdImageColorAllocate(im,82,209,24);
        gdImageColorAllocate(im,88,211,23);
        gdImageColorAllocate(im,93,214,22);
        gdImageColorAllocate(im,99,216,21);
        gdImageColorAllocate(im,105,218,20);
        gdImageColorAllocate(im,110,221,19);
        gdImageColorAllocate(im,116,223,18);
        gdImageColorAllocate(im,122,226,17);
        gdImageColorAllocate(im,128,228,16);
        gdImageColorAllocate(im,134,231,15);
        gdImageColorAllocate(im,140,233,14);
        gdImageColorAllocate(im,146,236,14);
        gdImageColorAllocate(im,151,238,13);
        gdImageColorAllocate(im,158,240,12);
        gdImageColorAllocate(im,164,242,11);
        gdImageColorAllocate(im,169,244,11);
        gdImageColorAllocate(im,174,246,10);
        gdImageColorAllocate(im,180,247,9);
        gdImageColorAllocate(im,185,249,9);
        gdImageColorAllocate(im,191,251,8);
        gdImageColorAllocate(im,197,252,8);
        gdImageColorAllocate(im,201,252,7);
        gdImageColorAllocate(im,206,252,7);
        gdImageColorAllocate(im,211,252,6);
        gdImageColorAllocate(im,216,252,5);
        gdImageColorAllocate(im,220,252,5);
        gdImageColorAllocate(im,225,252,4);
        gdImageColorAllocate(im,229,252,4);
        gdImageColorAllocate(im,233,252,3);
        gdImageColorAllocate(im,237,252,3);
        gdImageColorAllocate(im,241,252,3);
        gdImageColorAllocate(im,244,252,2);
        gdImageColorAllocate(im,248,252,2);
        gdImageColorAllocate(im,250,252,1);
        gdImageColorAllocate(im,253,252,0);
        gdImageColorAllocate(im,255,251,0);
        gdImageColorAllocate(im,255,250,0);
        gdImageColorAllocate(im,255,247,0);
        gdImageColorAllocate(im,255,245,0);
        gdImageColorAllocate(im,255,242,0);
        gdImageColorAllocate(im,255,238,0);
        gdImageColorAllocate(im,255,235,0);
        gdImageColorAllocate(im,255,232,0);
        gdImageColorAllocate(im,255,227,0);
        gdImageColorAllocate(im,255,222,0);
        gdImageColorAllocate(im,255,218,0);
        gdImageColorAllocate(im,255,213,0);
        gdImageColorAllocate(im,255,208,0);
        gdImageColorAllocate(im,255,203,0);
        gdImageColorAllocate(im,255,198,0);
        gdImageColorAllocate(im,255,192,0);
        gdImageColorAllocate(im,255,187,0);
        gdImageColorAllocate(im,255,181,0);
        gdImageColorAllocate(im,255,175,0);
        gdImageColorAllocate(im,255,169,0);
        gdImageColorAllocate(im,255,163,0);
        gdImageColorAllocate(im,255,156,0);
        gdImageColorAllocate(im,255,150,0);
        gdImageColorAllocate(im,255,144,0);
        gdImageColorAllocate(im,255,137,0);
        gdImageColorAllocate(im,255,131,0);
        gdImageColorAllocate(im,255,124,0);
        gdImageColorAllocate(im,255,118,0);
        gdImageColorAllocate(im,255,111,0);
        gdImageColorAllocate(im,255,105,0);
        gdImageColorAllocate(im,255,99,0);
        gdImageColorAllocate(im,255,93,0);
        gdImageColorAllocate(im,255,86,0);
        gdImageColorAllocate(im,255,81,0);
        gdImageColorAllocate(im,255,74,0);
        gdImageColorAllocate(im,255,68,0);
        gdImageColorAllocate(im,255,62,0);
        gdImageColorAllocate(im,255,57,0);
        gdImageColorAllocate(im,255,51,0);
        gdImageColorAllocate(im,255,45,0);
        gdImageColorAllocate(im,255,41,0);
        gdImageColorAllocate(im,255,36,0);
        gdImageColorAllocate(im,255,31,0);
        gdImageColorAllocate(im,255,26,0);
        gdImageColorAllocate(im,255,22,0);
        gdImageColorAllocate(im,255,18,0);
        gdImageColorAllocate(im,255,14,0);
        gdImageColorAllocate(im,255,10,0);
        gdImageColorAllocate(im,255,6,0);
        gdImageColorAllocate(im,255,4,0);
        gdImageColorAllocate(im,255,1,0);
        gdImageColorAllocate(im,255,0,0);
        gdImageColorAllocate(im,255,0,0);
        gdImageColorAllocate(im,255,0,0);
        gdImageColorAllocate(im,255,0,0);
        gdImageColorAllocate(im,255,0,0);
        gdImageColorAllocate(im,255,0,0);
        gdImageColorAllocate(im,255,0,0);
        gdImageColorAllocate(im,255,0,0);
        gdImageColorAllocate(im,255,1,1);
        gdImageColorAllocate(im,255,1,1);
        gdImageColorAllocate(im,255,2,2);
        gdImageColorAllocate(im,255,2,2);
        gdImageColorAllocate(im,255,3,3);
        gdImageColorAllocate(im,255,5,5);
        gdImageColorAllocate(im,255,6,6);
        gdImageColorAllocate(im,255,8,8);
        gdImageColorAllocate(im,255,11,11);
        gdImageColorAllocate(im,255,14,14);
        gdImageColorAllocate(im,255,18,18);
        gdImageColorAllocate(im,255,23,23);
        gdImageColorAllocate(im,255,29,29);
        gdImageColorAllocate(im,255,37,37);
        gdImageColorAllocate(im,255,46,46);
        gdImageColorAllocate(im,255,56,56);
        gdImageColorAllocate(im,255,67,67);
        gdImageColorAllocate(im,255,81,81);
        gdImageColorAllocate(im,255,94,94);
        gdImageColorAllocate(im,255,110,110);
        gdImageColorAllocate(im,255,126,126);
        gdImageColorAllocate(im,255,142,142);
        gdImageColorAllocate(im,255,159,159);
        gdImageColorAllocate(im,255,176,176);
        gdImageColorAllocate(im,255,193,193);
        gdImageColorAllocate(im,255,209,209);
        gdImageColorAllocate(im,255,224,224);
        gdImageColorAllocate(im,255,237,237);

        for (int y=0; y < m_ysize; ++y) {
            for (int x=0; x < m_xsize; ++x) {
                int n = y * m_xsize + x;
                gdImageSetPixel(im, x, y, array[n]);
            }
        }

        FILE *f = fopen(m_pngfilename, "w+");
        gdImagePng(im, f);
        gdImageDestroy(im);
        fclose(f);
        throw Osmium::Handler::StopReading();
    }

}; // class NodeDensityHandler

/* ================================================== */

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4)
    {
        std::cerr << "Usage: " << argv[0] << " OSMFILE OUTFILE [pngfile]\n\n";
        std::cerr << "  OSMFILE - OSM file of any type.\n";
        std::cerr << "  OUTFILE - node density index file.\n";
        std::cerr << "  pngfile - optional PNG heatmap.\n";
        exit(1);
    }

    Osmium::OSMFile infile(argv[1]);
    NodeDensityHandler handler(argv[2], (argc==3) ? NULL : argv[3]);
    Osmium::Input::read(infile, handler);

    google::protobuf::ShutdownProtobufLibrary();
}

