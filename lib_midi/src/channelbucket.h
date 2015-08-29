/********************** ls_channelbucket: LinnStrument ChannelBucket class ************************
Coming from the operating System for the LinnStrument (c) music controller by Roger Linn Design (www.rogerlinndesign.com).

Written by Roger Linn and Geert Bevin (http://gbevin.com).

LinnStrument Operating System is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License,
viewable at <http://creativecommons.org/licenses/by-sa/3.0/>.

This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
***************************************************************************************************
Handles the handing out of MIDI channels from a bucket of allowed channel numbers.

The available channels are added to the bucket in the beginning.

When a note needs a new channel, it just takes it. The channel will move to the bottom of the
bucket and will be reused when all the other channels have been taken also. This makes it
possible for the same channel number to be in use by different note, depending on the number of
channels that were added to the bucket and the polyphony of the notes that are being played.

At the same time, the bucket keeps track of which channels are taken and which channels are
released and free from any touches. This is achieved by dividing the bucket in two sections, an
upper section with all the released channels and a bottom section with all the channels that are
already taken.

When a note is released, it should release its channel. If the channel is still in use by another
note since the polyphony was exceeded, the channel will move to the bottom of the taken section.
If no other notes are using the channel, the channel will be moved to the bottom of the released
section, somewhere in the middle of the whole bucket.

This whole approach ensures that channels are reused as late as possible, while attempting to
prevent handing out the same channel for multiple notes. Postponing the reuse of a channel as much
as possible is important for sounds that have long releases.
**************************************************************************************************/

#ifndef CHANNELBUCKET_H_
#define CHANNELBUCKET_H_

typedef unsigned char byte;

class ChannelBucket {
public:
  ChannelBucket() {
    clear();
  }

  ~ChannelBucket() {}

  void add(byte channel) {
    // we can't add a MIDI channel that exceeds 16
    if (channel > 16) return;

    // offset the channel for a 0-based array
    channel -= 1;

    // don't add a channel several times
    if (next_[channel] != -1) return;

    // this is the first channel to be added to the bucket
    if (-1 == top_) {
      top_ = channel;
      previous_[channel] = channel;
      next_[channel] = channel;
      taken_[channel] = 0;
      bottomReleased_ = channel;
    }
    // add the channel to right after the current bottom-most released channel
    // and make the new channel the bottom-most released one
    else {
      previous_[channel] = bottomReleased_;
      previous_[next_[bottomReleased_]] = channel;

      next_[channel] = next_[bottomReleased_];
      next_[bottomReleased_] = channel;

      taken_[channel] = 0;
      bottomReleased_ = channel;
    }
  }

  byte take() {
    // return an invalid channel if none have been added
    if (-1 == top_) return 0;

    // get the channel at the top of the bucket
    byte channel = top_;

    // sink the channel to the bottom of the entire bucket, also crossing
    // the release/taken marker. Essentially the channel goes to the bottom
    // of the taken section. Since the channel was already at the top of the
    // bucket, this is very simply done by adjusting the top marker to the
    // next channel available
    top_ = next_[channel];

    // indicate that the channel was taken once more
    taken_[channel]++;

    // if this channel was the last of the released section, indicate that
    // the released marker has become invalid
    if (channel == bottomReleased_) {
      bottomReleased_ = -1;
    }

    // adjust for 1-base channel numbers
    return channel+1;
  }

  byte take(byte channel) {
    // we can't take a MIDI channel that exceeds 16, nor can we work with an empty bucket
    // we expect this channel to also be already in the bucket
    if (channel > 16 || -1 == top_ || -1 == next_[channel-1]) return 0;

    // offset the channel for a 0-based array
    channel -= 1;

    // indicate that the channel was taken one more time
    taken_[channel]++;

    // adjust the released section marker to either become invalid is this
    // valid the last channel that was released, or to shift one up in the
    // released section
    if (bottomReleased_ == channel) {
      if (top_ == channel) {
        bottomReleased_ = -1;
      }
      else {
        bottomReleased_ = previous_[channel];
      }
    }

    // if this channel was at the top, readjust the top marker to point to the next channel
    if (top_ == channel) {
      top_ = next_[channel];
    }

    // ensure that the channel goes to the bottom of the taken section
    extremize(channel);

    // adjust for 1-base channel numbers
    return channel+1;
  }

  void release(byte channel) {
    // we can't release a MIDI channel that exceeds 16, nor can we work with an empty bucket
    // we expect this channel to also be already in the bucket
    if (channel > 16 || -1 == top_ || -1 == next_[channel-1]) return;

    // offset the channel for a 0-based array
    channel -= 1;

    // indicate that the channel was taken one less time
    taken_[channel]--;

    // if the channel is still taken, ensure it goes to the bottom of the taken section
    if (taken_[channel] > 0) {
      extremize(channel);
      top_ = next_[channel];
    }
    // if this release was the one using the channel, ensure that the channel goes at the bottom of the released section
    else {
      // if the released section doesn't exist anymore (all channels were taken),
      // put the channel at the extremes of the bucket and mark it as being the new
      // top channel
      if (bottomReleased_ == -1) {
        extremize(channel);
        top_ = channel;
      }
      // put the channel at the bottom of the released section
      else {
        // however, if the channel happens to be at the top of the
        // taken section (right after the released section), we don't need to do anything
        // besides readjusting the marker of the released section
        if (next_[bottomReleased_] != channel) {

          // determine the edges of both the released and the taken sections
          int releasedEdge = bottomReleased_;
          int takenEdge = next_[bottomReleased_];

          // extract the channel from the bucket so that nothing is pointing
          // towards it anymore
          extract(channel);

          // re-insert the channel between the current released and taken sections
          previous_[channel] = releasedEdge;
          next_[releasedEdge] = channel;

          previous_[takenEdge] = channel;
          next_[channel] = takenEdge;
        }
      }

      // this channel is the last released one now
      bottomReleased_ = channel;
    }
  }

  // remove all channels from the bucket
  void clear() {
    top_ = -1;
    for (int ch = 0; ch < 16; ++ch) {
      previous_[ch] = -1;
      next_[ch] = -1;
      taken_[ch] = 0;
    }
  }

private:

  void extract(byte channel) {
    if (next_[channel] != -1) {
      next_[previous_[channel]] = next_[channel];
      previous_[next_[channel]] = previous_[channel];

      previous_[channel] = -1;
      next_[channel] = -1;
    }
  }

  void extremize(byte channel) {
    int bottom = previous_[top_];
    if (bottom == channel) {
      bottom = previous_[channel];
    }
    int top = top_;
    if (top == channel) {
      top = next_[channel];
    }

    extract(channel);

    previous_[channel] = bottom;
    next_[bottom] = channel;

    previous_[top] = channel;
    next_[channel] = top;
  }

  void debugBucket() {
    pic::logmsg() << "top=" << top_ << " bottomReleased=" << bottomReleased_;
    for (int ch = 0; ch < 16; ++ch) {
      pic::logmsg() << "channel=" << ch
        << " previous=" << previous_[ch]
        << " next=" << next_[ch]
        << " taken=" << (unsigned long)taken_[ch];
    }
  }

  int top_;            // the channel number of the top one in the bucket
  int previous_[16];   // the channel number of the previous channel in the bucket for each individual channel
  int next_[16];       // the channel number of the next channel in the bucket for each individual channel
  byte taken_[16];     // counts how many times each channel is still taken
  int bottomReleased_; // marks the bottom of the released section
};

#endif
