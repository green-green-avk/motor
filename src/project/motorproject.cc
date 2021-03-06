#include "motorproject.h"
#include "motorconfiguration.h"
#include "motormisc.h"
#include "motorexecutor.h"
#include "motorui.h"
#include "motordebugger.h"
#include "motorvcs.h"
#include "motormanager.h"

#include "kkfsys.h"

#include <dirent.h>

motorproject::motorproject(): gettextized(false) {
}

motorproject::motorproject(const projectname aname, INT aoptions) {
    gettextized = false;
    options = aoptions;
    absorb(aname);

    if(!load(aname)) {
	projectname::clear();
    }
}

motorproject::~motorproject() {
}

bool motorproject::load(const projectname aaname) {
    bool ret, tmplchanged = false;
    string sect, buf;
    ifstream f;
    projectname aname(aaname);

    if(aname.empty())
	return false;

    buf = aname.gettemplatename();
    vector<string> tl = manager.gettemplatelist();
    while(find(tl.begin(), tl.end(), buf) == tl.end())
	if(ui.notemplate(buf) != motorui::yes)
	    return false;

    if(tmplchanged = (buf != aname.gettemplatename()))
	aname.settemplate(buf);

    projectname::absorb(aaname);
    projectpaths::load(aname);
    projecttempl::load(aname);
    projectfiles::load(aname);
    projectdebug::load(aname);
    projectdesk::load(aname);

    f.open(getregfname().c_str());
    if(ret = f.is_open()) {
        while(getconf(sect, buf, f)) {
             if(sect == "gettextized") gettextized = buf == "1";
	     else if(sect == "makemode") {
                 makemode = buf == "automake" ?
                     motorproject::automake :
                     motorproject::manual;
             } else if(sect == "vcsname") {
	        vcs = motorvcs(vcsname = buf);
	     } else if(sect == "cflags") {
	        cflags = buf;
	     } else if(sect == "lflags") {
	        lflags = buf;
	     }
        }

        f.close();
    }

    if(!(options & LP_NOCHECK)) {
	if(!(ret = !access(rootdir.c_str(), X_OK))) {
	    ui.log(_("Unable to find the project root directory"));
	}
    }

    if(tmplchanged)
	save();

    chdir();
    projectname::save();
    return ret;
}

string motorproject::getprojfname() const {
    return rootdir + "/" + getname() + ".motor";
}

void motorproject::save() {
    bool found;
    vector<motorui::editfile> ef;
    vector< pair<string, string> > asettings;

    vector<motorui::editfile>::iterator ief;
    vector<motorfolder>::iterator ifold;
    vector<motorfile>::iterator ifile;
    vector<breakpoint>::iterator ibp;
    vector< pair<string, string> >::iterator ist, iost;
    vector<string>::iterator iw;

    ofstream f(getprojfname().c_str());

    if(f.is_open()) {
	ef = ui.geteditfiles();
	bpoints = debugger.getbreakpoints();
	watches = debugger.getwatches();

	f <<
	    "# This file is auto-generated by the Motor IDE and contains definition" << endl <<
	    "# of a project. For more information visit http://thekonst.net/motor" << endl << endl;

	f <<
	    "%version" << endl << projectname::getversion() << endl <<
	    "%template" << endl << projectname::gettemplatename() << endl;
	f <<
	    "%vcsname" << endl << getvcs() << endl <<
	    "%vcsroot" << endl << projectpaths::getvcsroot() << endl <<
	    "%vcsmodule" << endl << projectpaths::getvcsmodulename() << endl;
	f <<
	    "%arguments" << endl << projectdesk::getarguments() << endl;
	f <<
	    "%cflags" << endl << cflags << endl <<
	    "%lflags" << endl << lflags << endl;
	f <<
	    "%gettextized" << endl << (gettextized ? "1" : "0") << endl <<
            "%makemode" << endl << MAKEMODE_TO_STR(makemode) << endl;

	for(ifold = foldbegin(); ifold != foldend(); ifold++)
	for(ifile = ifold->begin(); ifile != ifold->end(); ifile++) {
	    f << "%file " << ifold->gettagname() << " " << ifile->getfname() << endl;

	    if(ifile->getbuild().param.empty()) {
		f << "1" << endl;
	    } else {
		motorfile::build b = ifile->getbuild();

		f << "param\t" << b.param << endl <<
		    "help\t" << b.help << endl <<
		    "default\t" << (b.def ? "1" : "0") << endl;
	    }
	}

	f << "%editor" << endl;
	for(ief = ef.begin(); ief != ef.end(); ief++) {
	    f << ief->fname << "\t" << ief->x << "\t" << ief->y << endl;
	}

	f << "%breakpoints" << endl;
	for(ibp = bpoints.begin(); ibp != bpoints.end(); ibp++) {
	    f << ibp->getfname() << "\t" << ibp->getline() << endl;
	}

	f << "%watch" << endl;
	for(iw = watches.begin(); iw != watches.end(); iw++) {
	    f << *iw << endl;
	}

	f << "%desktop" << endl;
	asettings = ui.getdesktop();

	for(ist = asettings.begin(); ist != asettings.end(); ist++) {
	    found = false;

	    for(iost = projectdesk::settings.begin(); !found && iost != projectdesk::settings.end(); iost++) {
		if(found = iost->first == ist->first) {
		    iost->second = ist->second;
		}
	    }

	    if(!found) projectdesk::settings.push_back(*ist);
	}

	for(iost = projectdesk::settings.begin(); iost != projectdesk::settings.end(); iost++) {
	    f << iost->first << "\t" << iost->second << endl;
	}

	projectname::save();
	f.close();

	mksubdirs(conf.getmotordir() + "/projects/");
	symlink(getprojfname().c_str(), getregfname().c_str());
    }
}

bool motorproject::close() {
    bool r;

    if(!(r = empty())) {
	save();

	if(r = ui.editcloseall()) {
	    tagbrowser.clear();
	    debugger.done();
	    debugger = motordebugger();
	}
    }

    return r;
}

void motorproject::getflags(string &acflags, string &alflags) const {
    acflags = cflags;
    alflags = lflags;
}

void motorproject::setflags(const string &acflags, const string &alflags) {
    setmodified(cflags != acflags || lflags != alflags);
    cflags = acflags;
    lflags = alflags;
}

bool motorproject::isgettextized() const {
    return gettextized;
}

void motorproject::setgettextized(bool a) {
    setmodified(gettextized != a);
    gettextized = a;
}

motorproject::makemodekind motorproject::getmakemode() const {
    return makemode;
}

void motorproject::setmakemode(motorproject::makemodekind amakemode) {
    setmodified(makemode != amakemode);
    makemode = amakemode;
}

void motorproject::chdir() {
    ::chdir(projectpaths::rootdir.c_str());
}

void motorproject::arrangebuildstuff() {
    DIR *d;
    struct dirent *de;
    vector<string> amake, emake, pdirs;
    vector<motorfile> mf;
    vector<motorfile>::iterator im;
    vector<string>::iterator is;
    string pname;

    if(getmakemode() == automake) {
	pdirs = projectfiles::extractdirectories();

	mf = getfiles_r("(\\/|^)Makefile\\.am$", motorfile::source);
	amake = filefind("(\\/|^)Makefile\\.am$", getrootdir(), FFIND_FILE);

	for(im = mf.begin(); im != mf.end(); im++) {
	    emake.push_back(transformfname(relative, im->getfname()));
	}

	/*
	*
	* Now we'll remove Makefiles that don't reside in project
	* directories from the list of newly added ones
	*
	*/

	for(is = amake.begin(); is != amake.end(); ) {
	    *is = transformfname(relative, *is);
	    pname = justpathname(*is);

	    if((pname != "") && (find(pdirs.begin(), pdirs.end(), pname) == pdirs.end())) {
		amake.erase(is);
		is = amake.begin();
	    } else {
		is++;
	    }
	}

	/*
	*
	* All we have to do now is just to perform appropriate add
	* and remove operations. Voila!
	*
	*/

	for(is = amake.begin(); is != amake.end(); is++) {
	    if(find(emake.begin(), emake.end(), *is) == emake.end()) {
	        addfile(*is, AF_VCS);
	    }
	}

	for(is = emake.begin(); is != emake.end(); is++) {
	    if(find(amake.begin(), amake.end(), *is) == amake.end()) {
		removefile(*is);
	    }
	}
    }
}

void motorproject::clean() {
    writemakefunc();
    getdisp();
    ui.setoutputblockmode(!fdmake);
    executor.runmake("clean");
    executor.clearvars();
}

void motorproject::regenerate() {
    writemakefunc();
    getdisp();
    ui.setoutputblockmode(!fdmake);

    if(executor.runmake("update")) {
	arrangebuildstuff();
    }

    executor.clearvars();
}

void motorproject::dist(const string &targetname, const string &adestdir) {
    destdir = adestdir;

    if(ui.autosave()) {
	ui.log(_("Generating a distribution package of the project.."));
	writemakefunc();
	getdisp();
	ui.setoutputblockmode(!fdmake);
	executor.runmake(targetname);
	executor.clearvars();
    }
}

bool motorproject::build() {
    bool ret;

    if(ret = ui.autosave()) {
	ui.log(_("Building the project.."));
	writemakefunc();
	getdisp();
	ui.setoutputblockmode(!fdcomp);
	ret = executor.runmake("build");
	executor.clearvars();
    }

    return ret;
}

void motorproject::populateparselist(pparamslist *parselist) {
    pparamslist *pl, *ppl;
    vector<motorfolder>::iterator ifold;
    vector<motorfile>::iterator ifile;
    motorproject cp;

    parser_svalue_reg(&svmakefile);

    projectname::populateparselist(parselist);
    projectpaths::populateparselist(parselist);
    projectdesk::populateparselist(parselist);

    pparamslist_add("compileroptions", cflags.c_str(), parselist);
    pparamslist_add("linkeroptions", lflags.c_str(), parselist);
    pparamslist_add("makefmode", MAKEMODE_TO_STR(makemode), parselist);
    pparamslist_add("author", conf.getuserfullname().c_str(), parselist);
    pparamslist_add("packoutdir", destdir.c_str(), parselist);

    if(gettextized) pparamslist_add("gettextized", "", parselist);

    for(ifold = foldbegin(); ifold != foldend(); ifold++)
	if(ifold->getcontentkind() == motorfile::project) {
    	    pl = pparamslist_add_list(ifold->gettagname().c_str(), parselist);

	    for(ifile = ifold->begin(); ifile != ifold->end(); ifile++) {
		if(!(cp = motorproject(projectname(ifile->getfname()))).empty()) {
		    ppl = pparamslist_add_list(ifile->getfname().c_str(), pl);
		    cp.populateparselist(ppl);

		    motorfile::build b = ifile->getbuild();
		    if(!b.param.empty()) {
			pparamslist_add("optional_param", b.param.c_str(), ppl);
			pparamslist_add("optional_help", b.help.c_str(), ppl);
			if(b.def) pparamslist_add("optional_enabled", "1", ppl);
		    }
		}
	    }
	} else {
	    pparamslist_add_array(ifold->gettagname().c_str(), 0, 0, parselist);

	    for(ifile = ifold->begin(); ifile != ifold->end(); ifile++)
		pparamslist_add_array_value(ifold->gettagname().c_str(),
		    ifile->getfname().c_str(), parselist);
	}

    chdir();
}

void motorproject::writemakefunc() {
    motorproject cp;
    vector<motorfolder>::iterator ifold;
    vector<motorfile>::iterator ifile;
    FILE *f;
    string tname, fname;
    pparamslist *p;

    fname = getrootdir() + "/Makefile.func";

    if(f = fopen(fname.c_str(), "w")) {
	p = pparamslist_create();
	populateparselist(p);
	tname = conf.gettemplatedir(gettemplatename()) + "/Makefile.func";
	strparse(tname.c_str(), tname.size(), p, f, PARSER_SRC_FILE, PARSER_FLOAD_MMAP);
	if(vcs.enabled())
	    if(getname() == project.getname())
		vcs.putmake(f, p);
	pparamslist_free(p);
        fclose(f);
    }

    for(ifold = foldbegin(); ifold != foldend(); ifold++)
	if(ifold->getcontentkind() == motorfile::project)
	    for(ifile = ifold->begin(); ifile != ifold->end(); ifile++) {
		cp = motorproject(ifile->getfname());
		if(!cp.empty()) cp.writemakefunc();
	    }

    chdir();
}

bool motorproject::create(INT options) {
    bool ret = true;

    if(getversion().empty()) {
        setversion("0.1");
    }

    ui.setdesktop(vector< pair<string, string> >());
    ui.log(_("Starting up the project.."));

    if(!getvcs().empty() && !getvcsroot().empty() && !getvcsmodulename().empty()) {
	vcs = motorvcs(getvcs());
	vcs.checkout();
        ret = vcs.good();
    }

    if(ret) {
	if(options & CR_GENERATE_SOURCE) generate();
	if(options & CR_GNU_DOC) addgnudoc();

	checkautomake();
	import("", getrootdir());

	writemakefunc();
	getdisp();
	ui.setoutputblockmode(!fdmake);

	if(ret = executor.runmake("start")) {
	    arrangebuildstuff();
	    if(ret = onprojectstart()) {
		save();
		addfile(getname() + ".motor");
		ui.logf(_("Project %s has been created"), getname().c_str());
	    }
	}

	executor.clearvars();
    }

    return ret;
}

void motorproject::checkautomake() {
    vector<sourcetemplate>::iterator i;

    if(getmakemode() == automake)
    if(access("configure.in", F_OK)) {
        i = find(sourcetemplates.begin(), sourcetemplates.end(), "configure.in");
        if(i != sourcetemplates.end()) i->generate();
    }
}

bool motorproject::onprojectstart() {
    bool r = true;

    if(!access(getprojfname().c_str(), R_OK)) {
	symlink(getprojfname().c_str(), getregfname().c_str());
	motorproject np(getname());
	unlink(getregfname().c_str());

	bool backup = false;

	if(!np.empty()) {
	    switch(ui.askf("YNC", _("There is a motor project file %s in the source, use it?"), justfname(getprojfname()).c_str())) {
		case motorui::yes: load(getname()); break;
		case motorui::cancel: r = false; break;
		case motorui::no: backup = true; break;
	    }
	} else {
	    backup = true;
	}

	if(backup) {
	    rename(getprojfname().c_str(), (getprojfname() + "~").c_str());
	}
    }

    return r;
}

void motorproject::settemplate(const string &atemplate) {
    projectname::settemplate(atemplate);
    projecttempl::load(*((projectname *) this));
    projectfiles::load(*((projectname *) this));
}

void motorproject::generate() {
    vector<sourcetemplate>::iterator i;

    for(i = sourcetemplates.begin(); i != sourcetemplates.end(); i++) {
	if(i->getfname() != "configure.in") {
	    i->generate();
	} else if(getmakemode() == automake) {
	    if(access("configure.in", F_OK)) {
		chdir();
		i->generate();
	    }
	}
    }
}

bool motorproject::remove() {
    bool rc;
    motorui::askresult ar;

    ui.autosave();
    close();

    ar = ui.askf("CYN", _("Delete all files of the project %s as well?"), getname().c_str());

    if(rc = (ar != motorui::cancel)) {
	unlink(getregfname().c_str());

	if(ar == motorui::yes) {
	    ::chdir(getenv("HOME"));
	    if(!fork()) {
	        execlp("/bin/rm", "/bin/rm", "-rf", getrootdir().c_str(), 0);
    		_exit(0);
	    }
	}

	ui.logf(_("Project %s has been removed"), getname().c_str());

	if(getname() == project.getname()) {
	    project.clear();
	}

	projectname::clear();
    }

    return rc;
}

void motorproject::import(const string &mask, const string &root, INT options) {
    DIR *d;
    regex_t r;
    struct dirent *de;
    struct stat st;
    string fname;
    static string toproot;
    vector<motorfolder>::iterator ifold;

    if(!regcomp(&r, mask.c_str(), REG_EXTENDED)) {
	if(d = opendir(root.c_str())) {
	    while(de = readdir(d))
	    if(de->d_name[0] != '.') {
		fname = de->d_name;

		if(getmakemode() == automake)
		if(fname == "Makefile" || fname == "Makefile.in")
		    continue;

		fname.insert(0, root + "/");

		if(!stat(fname.c_str(), &st)) {
		    fname = transformfname(relative, fname);

		    if(S_ISDIR(st.st_mode)) {
			import(mask, fname, options);
		    } else if(S_ISREG(st.st_mode)) {
			if(!regexec(&r, fname.c_str(), 0, 0, 0))
			    addfile(fname, options);
		    }
		}
	    }

	    closedir(d);
	}

	regfree(&r);
    }
}

void motorproject::runtarget(const string &targetname) {
    writemakefunc();
    executor.setvar("MOTOR_TARGET", targetname);
    getdisp();
    ui.setoutputblockmode(!fdmake);
    executor.runmake("target");
    executor.clearvars();
}

char *motorproject::svmakefile(const char *value, const char *key) {
    char *ret = 0;
    string st;

    if(key) {
        if(!strcmp(key, "extractfname")) {
	    st = justfname(value);
	    ret = strdup(st.c_str());
        }
    }

    return ret;
}

void motorproject::setvcs(const string &avcsname) {
    setmodified(vcsname != avcsname);
    vcs = motorvcs(vcsname = avcsname);
}

string motorproject::getvcs() const {
    return vcsname;
}

bool motorproject::execvcs(const string &action) {
    bool r;
    parserule *pr;

    writemakefunc();
    getdisp();
    ui.setoutputblockmode(!fdvcs);

    for(bool fin = false; !fin; ) {
	executor.runmake("vcs_" + action);
	r = fin = !vcs.iserror(pr, executor.getlastbuf());
	if(!fin) {
	    ui.log(pr->getvalue());

	    fin = ui.askf("YN", _("VCS %s command wasn't successful. Retry?"),
		action.c_str()) == motorui::no;

	    if(fin)
	    if(action == "addfile") {
		r = ui.askf("YN", _("Add the file to the project locally anyway?"),
		    action.c_str()) == motorui::yes;
	    }
	}
    }

    executor.clearvars();
    ui.reloadeditfiles();
    return r;
}

bool motorproject::runtags() {
    bool r;
    string scope;

    writemakefunc();
    getdisp();
    ui.setoutputblockmode(true);

    switch(tagbrowser.getscope()) {
	case motortagbrowser::File: scope = "file"; break;
	case motortagbrowser::Project: scope = "project"; break;
	case motortagbrowser::Everything: scope = "all"; break;
    }

    executor.setvar("MOTOR_TAGS", scope);
    r = executor.runmake("tags");
    executor.clearvars();

    return r;
}

void motorproject::getdisp() {
    conf.getdisplay(fdmake, fdcomp, fdvcs);
}

void motorproject::addgnudoc() {
    writemakefunc();
    getdisp();
    ui.setoutputblockmode(!fdmake);
    executor.runmake("gnudoc");
    executor.clearvars();
}

bool motorproject::addfile(const motorfile afile, INT options) {
    bool r;
    vector<motorfolder>::iterator ifold;

    for(r = false, ifold = foldbegin(); !r && (ifold != foldend()); ifold++)
	if(ifold->getcontentkind() == motorfile::source)
	    r = ifold->addfile(afile, options);

    return r;
}

void motorproject::removefile(const motorfile afile, motorfile::filekind akind) {
    vector<motorfolder>::iterator ifold;

    for(ifold = foldbegin(); ifold != foldend(); ifold++)
	if(ifold->getcontentkind() == akind)
	if(find(ifold->begin(), ifold->end(), afile) != ifold->end())
	    ifold->removefile(afile);
}
