#include "resample.h"
#include "soxr.h"
#include <QDebug>
#include "sox.h"

//http://qaru.site/questions/2153780/downsampling-a-wav-file-using-libsox


resample::resample(QObject *parent) : QObject(parent)
{
}

//void resample::start(short *input)
bool resample::getResample(QString originalFileName, QString resampleFileName)
{
    int argc;
    char * argv[5];
    static sox_format_t * in, * out; /* input and output files */
    sox_effects_chain_t * chain;
    sox_effect_t * e;
    char * args[10];
    sox_signalinfo_t interm_signal; /* @ intermediate points in the chain. */
    sox_encodinginfo_t out_encoding = {
      SOX_ENCODING_SIGN2,
      24,
      0,
      sox_option_default,
      sox_option_default,
      sox_option_default,
      sox_false
    };
    sox_signalinfo_t out_signal = {
      44100,
      1,
      0,
      0,
      NULL
    };

    argc = 3;
    argv[1] = originalFileName.toUtf8().data();
    argv[2] = resampleFileName.toUtf8().data();

    qDebug()<<argv[2];

    assert(argc == 3);
    assert(sox_init() == SOX_SUCCESS);
    assert(in = sox_open_read(argv[1], NULL, NULL, NULL));
    assert(out = sox_open_write(argv[2], &out_signal, &out_encoding, NULL, NULL, NULL));

    chain = sox_create_effects_chain(&in->encoding, &out->encoding);

    interm_signal = in->signal; /* NB: deep copy */

    e = sox_create_effect(sox_find_effect("input"));
    args[0] = (char *)in, assert(sox_effect_options(e, 1, args) == SOX_SUCCESS);
    assert(sox_add_effect(chain, e, &interm_signal, &in->signal) == SOX_SUCCESS);
    free(e);

    if (in->signal.rate != out->signal.rate) {
      e = sox_create_effect(sox_find_effect("rate"));
      assert(sox_effect_options(e, 0, NULL) == SOX_SUCCESS);
      assert(sox_add_effect(chain, e, &interm_signal, &out->signal) == SOX_SUCCESS);
      free(e);
    }

    if (in->signal.channels != out->signal.channels) {
      e = sox_create_effect(sox_find_effect("channels"));
      assert(sox_effect_options(e, 0, NULL) == SOX_SUCCESS);
      assert(sox_add_effect(chain, e, &interm_signal, &out->signal) == SOX_SUCCESS);
      free(e);
    }

    e = sox_create_effect(sox_find_effect("output"));
    args[0] = (char *)out, assert(sox_effect_options(e, 1, args) == SOX_SUCCESS);
    assert(sox_add_effect(chain, e, &interm_signal, &out->signal) == SOX_SUCCESS);
    free(e);

    sox_flow_effects(chain, NULL, NULL);

    sox_delete_effects_chain(chain);
    sox_close(out);
    sox_close(in);
    sox_quit();

}

short* resample::getSample()
{
    return output;
}
