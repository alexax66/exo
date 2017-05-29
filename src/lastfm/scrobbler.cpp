/* ========================================================================
*    Copyright (C) 2013-2017 Blaze <blaze@vivaldi.net>
*
*    This file is part of eXo.
*
*    eXo is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    eXo is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with eXo.  If not, see <http://www.gnu.org/licenses/>.
* ======================================================================== */

#include "config.h"

#include <QSettings>
#include <lastfm5/ws.h>
#include <lastfm5/Audioscrobbler.h>
#include <lastfm5/Track.h>

#include "core/playerinterface.h"
#include "scrobbler.h"

const char* Scrobbler::apiKey = "75ca28a33e04af35b315c086736a6e7c";
const char* Scrobbler::secret = "a341d91dcf4b4ed725b72f27f1e4f2ef";

QPointer<Scrobbler> Scrobbler::object = nullptr;

Scrobbler::Scrobbler(QObject *parent) : QObject(parent) {
    if(object)
        qFatal("Scrobbler: only one instance is allowed");
    object = this;
    QSettings settings;
    lastfm::ws::Username = settings.value(
                QLatin1String("scrobbler/login")).toString();
    lastfm::ws::SessionKey = settings.value(
                QLatin1String("scrobbler/sessionkey")).toString();
    lastfm::ws::ApiKey = apiKey;
    lastfm::ws::SharedSecret = secret;
    as = new lastfm::Audioscrobbler(QLatin1String("eXo"));
    PlayerInterface* player = PlayerInterface::self();
    connect(player, &PlayerInterface::trackChanged, this, &Scrobbler::init);
    connect(player, &PlayerInterface::trackListened, this, &Scrobbler::submit);
}

Scrobbler::~Scrobbler() {
    delete as;
}

void Scrobbler::init(const QString& artist, const QString& title,
                     int totalSec) {
    lastfm::MutableTrack t;
    t.setArtist(artist);
    t.setTitle(title);
    t.setDuration(totalSec);
    as->nowPlaying(t);
}

void Scrobbler::submit(const QString& artist, const QString& title,
                       const QString& album, int totalSec) {
    lastfm::MutableTrack t;
    t.setArtist(artist);
    t.setTitle(title);
    t.setAlbum(album);
    t.setDuration(totalSec);
    t.stamp(); //sets track start time
    as->cache(t);
    as->submit();
}
