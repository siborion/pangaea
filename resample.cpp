#include "resample.h"
#include "soxr.h"
#include <QDebug>
#include "sox.h"

//http://qaru.site/questions/2153780/downsampling-a-wav-file-using-libsox


resample::resample(QObject *parent) : QObject(parent)
{
}

void resample::start(short *input)
{
    static sox_format_t * in, * out;
    char * argv[5];

    argv[1] = "24.wav";
    argv[2] = "24_3.wav";

    sox_effects_chain_t *chain;
    sox_effect_t * e;

    assert(sox_init() == SOX_SUCCESS);
    assert(in = sox_open_read(argv[1], NULL, NULL, NULL));
    out= (sox_format_t *) malloc(sizeof (sox_format_t));
    memcpy(out, in, sizeof (sox_format_t));
    out->encoding.encoding = SOX_ENCODING_ULAW;
    out->encoding.bits_per_sample=8;
    out->signal.rate = 8000;
    out->signal.precision = 8;
    out->signal.length = SOX_UNSPEC;
    assert(out = sox_open_write(argv[2], &out->signal, &out->encoding, NULL, NULL, NULL));

    chain = sox_create_effects_chain(&in->encoding, &out->encoding);

    e = sox_create_effect(sox_find_effect("input"));
    assert(sox_add_effect(chain, e, &in->signal, &in->signal) == SOX_SUCCESS);
    free(e);

//    out->signal.rate = 8000;
//    in->signal.rate = 16000;

    if (in->signal.rate != out->signal.rate)
    {
        e = sox_create_effect(sox_find_effect("rate"));
        assert(sox_effect_options(e, 0, NULL) == SOX_SUCCESS);
        e->handler.flags |= SOX_EFF_LENGTH;
        assert(sox_add_effect(chain, e, &in->signal, &out->signal) == SOX_SUCCESS);
        free(e);
    }

    sox_delete_effects_chain(chain);
    sox_close(out);
    sox_close(in);
    sox_quit();
}

short* resample::getSample()
{
    return output;
}
