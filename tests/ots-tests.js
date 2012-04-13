var frame = {
  go: function(url) {
    var dfd = $.Deferred();

    $('#frame').attr('src', url);
    $('#frame').load(function() {
      dfd.resolve(window.frames[0].window.jQuery);
    });

    return dfd.promise();
  },

  load: function() {
    var dfd = $.Deferred();

    $('#frame').load(function() {
      dfd.resolve(window.frames[0].window.jQuery);
    });

    return dfd.promise();
  }
};

module("Navigation menu test")
var url = 'http://localhost:5103/ots_nacl/ots_nacl.html#home';

asyncTest('check if selected item in navigation menu match with href', function() {
  $.when( frame.go(url) ).then(function(_$) {
    var resAnchor = _$('header nav [class=selected] a');
    var anchorName = $('iframe').attr('src');
    
    if(anchorName && anchorName.indexOf('#')) {
      anchorName = anchorName.split('#')[1];
    }
    ok(undefined !== resAnchor, "no anchor in navigation menu");
    
    var resAnchorName = _$(resAnchor).attr("href");
    ok(undefined !== resAnchorName, "no anchor href attribute in navigation menu");
    
    try {
      ok(resAnchorName.length > 0, "anchor name length in navigation menu is 0");
      equal(resAnchorName.substr(1), anchorName, 'anchor names does not match');
    } catch (error) {
      ok(1 == 0, error);
    };
    
    start();
  });
});

/*
module("Summarization test");

var article = "Ok, \n there's no way to do this gracefully, so I won't even try. I'm going to \njust hunker down for some really impressive extended flaming, and my \nasbestos underwear is firmly in place, and extremely uncomfortable.\n\n  I want to make it clear that DRM is perfectly ok with Linux!\n\nThere, I've said it. I'm out of the closet. So bring it on...\n\nI've had some private discussions with various people about this already,\nand I do realize that a lot of people want to use the kernel in some way\nto just make DRM go away, at least as far as Linux is concerned. Either by\nsome policy decision or by extending the GPL to just not allow it.\n\nIn some ways the discussion was very similar to some of the software\npatent related GPL-NG discussions from a year or so ago: \"we don't like\nit, and we should change the license to make it not work somehow\". \n\nAnd like the software patent issue, I also don't necessarily like DRM\nmyself, but I still ended up feeling the same: I'm an \"Oppenheimer\", and I\nrefuse to play politics with Linux, and I think you can use Linux for\nwhatever you want to - which very much includes things I don't necessarily\npersonally approve of.\n\nThe GPL requires you to give out sources to the kernel, but it doesn't\nlimit what you can _do_ with the kernel. On the whole, this is just\nanother example of why rms calls me \"just an engineer\" and thinks I have\nno ideals.\n\n[ Personally, I see it as a virtue - trying to make the world a slightly\n  better place _without_ trying to impose your moral values on other \n  people. You do whatever the h*ll rings your bell, I'm just an engineer \n  who wants to make the best OS possible. ]\n\nIn short, it's perfectly ok to sign a kernel image - I do it myself\nindirectly every day through the kernel.org, as kernel.org will sign the\ntar-balls I upload to make sure people can at least verify that they came\nthat way. Doing the same thing on the binary is no different: signing a\nbinary is a perfectly fine way to show the world that you're the one\nbehind it, and that _you_ trust it.\n\nAnd since I can imaging signing binaries myself, I don't feel that I can\ndisallow anybody else doing so.\n\nAnother part of the DRM discussion is the fact that signing is only the \nfirst step: _acting_ on the fact whether a binary is signed or not (by \nrefusing to load it, for example, or by refusing to give it a secret key) \nis required too.\n\nBut since the signature is pointless unless you _use_ it for something,\nand since the decision how to use the signature is clearly outside of the\nscope of the kernel itself (and thus not a \"derived work\" or anything like\nthat), I have to convince myself that not only is it clearly ok to act on\nthe knowledge of whather the kernel is signed or not, it's also outside of\nthe scope of what the GPL talks about, and thus irrelevant to the license.\n\nThat's the short and sweet of it. I wanted to bring this out in the open, \nbecause I know there are people who think that signed binaries are an act \nof \"subversion\" (or \"perversion\") of the GPL, and I wanted to make sure \nthat people don't live under mis-apprehension that it can't be done.\n\nI think there are many quite valid reasons to sign (and verify) your\nkernel images, and while some of the uses of signing are odious, I don't\nsee any sane way to distinguish between \"good\" signers and \"bad\" signers.\n\nComments? I'd love to get some real discussion about this, but in the end \nI'm personally convinced that we have to allow it.\n\nBtw, one thing that is clearly _not_ allowed by the GPL is hiding private\nkeys in the binary. You can sign the binary that is a result of the build\nprocess, but you can _not_ make a binary that is aware of certain keys\nwithout making those keys public - because those keys will obviously have\nbeen part of the kernel build itself.\n\nSo don't get these two things confused - one is an external key that is\napplied _to_ the kernel (ok, and outside the license), and the other one\nis embedding a key _into_ the kernel (still ok, but the GPL requires that\nsuch a key has to be made available as \"source\" to the kernel).\n\n			Linus";
var summarized_article = " ]\n\nIn short, it's perfectly ok to sign a kernel image - I do it myself\nindirectly every day through the kernel.org, as kernel.org will sign the\ntar-balls I upload to make sure people can at least verify that they came\nthat way.\n\nI think there are many quite valid reasons to sign (and verify) your\nkernel images, and while some of the uses of signing are odious, I don't\nsee any sane way to distinguish between \"good\" signers and \"bad\" signers. You can sign the binary that is a result of the build\nprocess, but you can _not_ make a binary that is aware of certain keys\nwithout making those keys public - because those keys will obviously have\nbeen part of the kernel build itself.\n\nSo don't get these two things confused - one is an external key that is\napplied _to_ the kernel (ok, and outside the license), and the other one\nis embedding a key _into_ the kernel (still ok, but the GPL requires that\nsuch a key has to be made available as \"source\" to the kernel).";

asyncTest('check if summarized articles match', function() {
  $.when( frame.load() ).then(function(_$) {
    _$("#input_text").val(article);
    _$("#page-submit-button").click();
    console.log(_$("#input_text").val());
    deepEqual(summarized_article, _$("#output_text").val(), "Summarized article and generated summarized article not same");
    start();
  });
});
*/

