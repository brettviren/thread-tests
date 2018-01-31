#!/usr/bin/env python
import os
import sys
import json

from jinja2 import Environment, FileSystemLoader


def render(out, template, **params):
    path = os.path.realpath(template)
    base = os.path.dirname(path)
    rel = os.path.basename(path)
    env = Environment(loader=FileSystemLoader(base))
    env.globals.update(zip=zip)
    tmplobj = env.get_template(rel)
    text = tmplobj.render(**params)
    open(out,'w').write(text)



if __name__ == '__main__':
    tmplfile = sys.argv[1]
    outfile = "/dev/stdout"
    render(outfile, tmplfile, hosts=[json.loads(open(one).read()) for one in sys.argv[2:]])
    
